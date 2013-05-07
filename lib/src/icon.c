/*
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <libgen.h>

#include <dlog.h>
#include <glib.h>
#include <db-util.h>
#include <vconf.h>
#include <vconf-keys.h>

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>

#include "shortcut_internal.h"
#include "shortcut.h"
#include "dlist.h"



static struct info {
	int fd;
	int (*init_cb)(int status, void *data);
	void *cbdata;
	int initialized;

	const char *utility_socket;

	struct dlist *pending_list;
} s_info = {
	.fd = -1,
	.init_cb = NULL,
	.cbdata = NULL,
	.initialized = 0,

	.utility_socket = "/tmp/.utility.service",
	.pending_list = NULL,
};



struct request_item {
	result_cb_t result_cb;
	void *data;
};



struct pending_item {
	struct request_item *item;
	struct packet *packet;
};



struct block {
	unsigned int idx;

	char *type;
	char *part;
	char *data;
	char *option;
	char *id;
	char *file;
	char *target_id;
};



struct shortcut_icon {
	struct shortcut_desc *desc;
	char *layout;
	char *output;
	char *descfile;
	char *group;
	int size_type;
};



struct shortcut_desc {
	FILE *fp;
	int for_pd;
	char *filename;

	unsigned int last_idx;

	struct dlist *block_list;
};



static int disconnected_cb(int handle, void *data)
{
	if (s_info.fd != handle)
		return 0;

	ErrPrint("Disconnected\n");
	s_info.fd = -1;
	s_info.init_cb = NULL;
	s_info.cbdata = NULL;
	s_info.initialized = 0;
	return 0;
}



static inline struct shortcut_desc *shortcut_icon_desc_open(const char *filename)
{
	struct shortcut_desc *handle;

	handle = calloc(1, sizeof(*handle));
	if (!handle) {
		ErrPrint("Error: %s\n", strerror(errno));
		return NULL;
	}

	handle->filename = strdup(filename);
	if (!handle->filename) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handle);
		return NULL;
	}

	handle->fp = fopen(handle->filename, "w+");
	if (!handle->fp) {
		ErrPrint("Failed to open a file: %s\n", strerror(errno));
		free(handle->filename);
		free(handle);
		return NULL;
	}

	return handle;
}



static inline int shortcut_icon_desc_close(struct shortcut_desc *handle)
{
	struct dlist *l;
	struct dlist *n;
	struct block *block;

	if (!handle)
		return -EINVAL;

	DbgPrint("Close and flush\n");
	dlist_foreach_safe(handle->block_list, l, n, block) {
		handle->block_list = dlist_remove(handle->block_list, l);

		DbgPrint("{\n");
		fprintf(handle->fp, "{\n");
		if (block->type) {
			fprintf(handle->fp, "type=%s\n", block->type);
			DbgPrint("type=%s\n", block->type);
		}

		if (block->part) {
			fprintf(handle->fp, "part=%s\n", block->part);
			DbgPrint("part=%s\n", block->part);
		}

		if (block->data) {
			fprintf(handle->fp, "data=%s\n", block->data);
			DbgPrint("data=%s\n", block->data);
		}

		if (block->option) {
			fprintf(handle->fp, "option=%s\n", block->option);
			DbgPrint("option=%s\n", block->option);
		}

		if (block->id) {
			fprintf(handle->fp, "id=%s\n", block->id);
			DbgPrint("id=%s\n", block->id);
		}

		if (block->target_id) {
			fprintf(handle->fp, "target=%s\n", block->target_id);
			DbgPrint("target=%s\n", block->target_id);
		}

		fprintf(handle->fp, "}\n");
		DbgPrint("}\n");

		free(block->type);
		free(block->part);
		free(block->data);
		free(block->option);
		free(block->id);
		free(block->target_id);
		free(block);
	}

	fclose(handle->fp);
	free(handle->filename);
	free(handle);
	return 0;
}



static inline int shortcut_icon_desc_set_id(struct shortcut_desc *handle, int idx, const char *id)
{
	struct dlist *l;
	struct block *block;

	dlist_foreach(handle->block_list, l, block) {
		if (block->idx == idx) {
			if (strcasecmp(block->type, SHORTCUT_ICON_TYPE_SCRIPT)) {
				ErrPrint("Invalid block is used\n");
				return SHORTCUT_ERROR_INVALID;
			}

			free(block->target_id);
			block->target_id = NULL;

			if (!id || !strlen(id))
				return SHORTCUT_SUCCESS;

			block->target_id = strdup(id);
			if (!block->target_id) {
				ErrPrint("Heap: %s\n", strerror(errno));
				return SHORTCUT_ERROR_MEMORY;
			}

			return SHORTCUT_SUCCESS;
		}
	}

	return SHORTCUT_ERROR_INVALID;
}
/*!
 * \return idx
 */



static inline int shortcut_icon_desc_add_block(struct shortcut_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option)
{
	struct block *block;

	if (!handle || !type)
		return SHORTCUT_ERROR_INVALID;

	if (!part)
		part = "";

	if (!data)
		data = "";

	block = calloc(1, sizeof(*block));
	if (!block) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_MEMORY;
	}

	block->type = strdup(type);
	if (!block->type) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block);
		return SHORTCUT_ERROR_MEMORY;
	}

	block->part = strdup(part);
	if (!block->part) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block->type);
		free(block);
		return SHORTCUT_ERROR_MEMORY;
	}

	block->data = strdup(data);
	if (!block->data) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(block->type);
		free(block->part);
		free(block);
		return SHORTCUT_ERROR_MEMORY;
	}

	if (option) {
		block->option = strdup(option);
		if (!block->option) {
			ErrPrint("Heap: %s\n", strerror(errno));
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return SHORTCUT_ERROR_MEMORY;
		}
	}

	if (id) {
		block->id = strdup(id);
		if (!block->id) {
			ErrPrint("Heap: %s\n", strerror(errno));
			free(block->option);
			free(block->data);
			free(block->type);
			free(block->part);
			free(block);
			return SHORTCUT_ERROR_MEMORY;
		}
	}

	block->idx = handle->last_idx++;
	handle->block_list = dlist_append(handle->block_list, block);
	return block->idx;
}



static int icon_request_cb(pid_t pid, int handle, const struct packet *packet, void *data)
{
	struct request_item *item = data;
	int ret;

	if (!packet) {
		ret = -EFAULT;
		DbgPrint("Disconnected?\n");
	} else {
		if (packet_get(packet, "i", &ret) != 1) {
			DbgPrint("Invalid packet\n");
			ret = -EINVAL;
		}
	}

	if (item->result_cb)
		item->result_cb(ret, pid, item->data);

	free(item);
	return 0;
}



static inline int make_connection(void)
{
	int ret;
	static struct method service_table[] = {
		{
			.cmd = NULL,
			.handler = NULL,
		},
	};

	s_info.fd = com_core_packet_client_init(s_info.utility_socket, 0, service_table);
	if (s_info.fd < 0) {
		ret = SHORTCUT_ERROR_COMM;

		if (s_info.init_cb)
			s_info.init_cb(ret, s_info.cbdata);
	} else {
		struct dlist *l;
		struct dlist *n;
		struct pending_item *pend;

		if (s_info.init_cb)
			s_info.init_cb(SHORTCUT_SUCCESS, s_info.cbdata);

		dlist_foreach_safe(s_info.pending_list, l, n, pend) {
			s_info.pending_list = dlist_remove(s_info.pending_list, l);

			ret = com_core_packet_async_send(s_info.fd, pend->packet, 0.0f, icon_request_cb, pend->item);
			packet_destroy(pend->packet);
			if (ret < 0) {
				ErrPrint("ret: %d\n", ret);
				if (pend->item->result_cb)
					pend->item->result_cb(ret, getpid(), pend->item->data);
				free(pend->item);
			}

			free(pend);
		}

		ret = SHORTCUT_SUCCESS;
	}

	return ret;
}



static void master_started_cb(keynode_t *node, void *user_data)
{
	int state = 0;

	if (vconf_get_bool(VCONFKEY_MASTER_STARTED, &state) < 0)
		ErrPrint("Unable to get \"%s\"\n", VCONFKEY_MASTER_STARTED);

	if (state == 1 && make_connection() == SHORTCUT_SUCCESS) {
		int ret;
		ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
		DbgPrint("Ignore VCONF [%d]\n", ret);
	}
}



EAPI int shortcut_icon_service_init(int (*init_cb)(int status, void *data), void *data)
{
	int ret;

	if (s_info.fd >= 0)
		return -EALREADY;

	if (s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	s_info.init_cb = init_cb;
	s_info.cbdata = data;

	ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL);
	if (ret < 0)
		ErrPrint("Failed to add vconf for service state [%d]\n", ret);
	else
		DbgPrint("vconf is registered\n");

	master_started_cb(NULL, NULL);
	return 0;
}



EAPI int shortcut_icon_service_fini(void)
{
	if (s_info.initialized) {
		com_core_del_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
		s_info.initialized = 0;
	}

	if (s_info.fd < 0)
		return -EINVAL;

	com_core_packet_client_fini(s_info.fd);
	s_info.init_cb = NULL;
	s_info.cbdata = NULL;
	s_info.fd = -1;
	return 0;
}



EAPI struct shortcut_icon *shortcut_icon_request_create(int size_type, const char *output, const char *layout, const char *group)
{
	struct shortcut_icon *handle;
	int len;

	if (!layout) {
		DbgPrint("Using default icon layout\n");
		layout = DEFAULT_ICON_LAYOUT;
	}

	if (!group) {
		DbgPrint("Using default icon group\n");
		group = DEFAULT_ICON_GROUP;
	}

	handle = malloc(sizeof(*handle));
	if (!handle) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	len = strlen(output) + strlen(".desc") + 1;
	handle->descfile = malloc(len);
	if (!handle->descfile) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handle);
		return NULL;
	}

	handle->layout = strdup(layout);
	if (!handle->layout) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handle->descfile);
		free(handle);
		return NULL;
	}

	handle->group = strdup(group);
	if (!handle->group) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handle->descfile);
		free(handle->layout);
		free(handle);
		return NULL;
	}

	handle->output = strdup(output);
	if (!handle->output) {
		ErrPrint("Heap: %s\n", strerror(errno));
		free(handle->group);
		free(handle->layout);
		free(handle->descfile);
		free(handle);
		return NULL;
	}

	snprintf(handle->descfile, len, "%s.desc", output);

	handle->desc = shortcut_icon_desc_open(handle->descfile);
	if (!handle->desc) {
		ErrPrint("Uanble to open desc\n");
		free(handle->output);
		free(handle->group);
		free(handle->layout);
		free(handle->descfile);
		free(handle);
		return NULL;
	}

	handle->size_type = size_type;
	return handle;
}



EAPI int shortcut_icon_request_set_info(struct shortcut_icon *handle, const char *id, const char *type, const char *part, const char *data, const char *option, const char *subid)
{
	int idx;

	idx = shortcut_icon_desc_add_block(handle->desc, id, type, part, data, option);
	if (subid && idx >= 0)
		shortcut_icon_desc_set_id(handle->desc, idx, subid);

	return idx;
}



EAPI int shortcut_icon_request_send_and_destroy(struct shortcut_icon *handle, result_cb_t result_cb, void *data)
{
	int ret;
	struct packet *packet;
	struct request_item *item;

	DbgPrint("Request and Destroy\n");
	shortcut_icon_desc_close(handle->desc);

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		ret = -ENOMEM;
	} else {
		item->result_cb = result_cb;
		item->data = data;

		packet = packet_create("icon_create", "sssis", handle->layout, handle->group, handle->descfile, handle->size_type, handle->output);
		if (!packet) {
			ErrPrint("Failed to create a packet\n");
			free(item);
			ret = -EFAULT;
			goto out;
		}

		if (s_info.fd >= 0 && !s_info.pending_list) {
			ret = com_core_packet_async_send(s_info.fd, packet, 0.0f, icon_request_cb, item);
			packet_destroy(packet);
			if (ret < 0) {
				ErrPrint("ret: %d\n", ret);
				free(item);
			}
			DbgPrint("Request is sent\n");
		} else {
			struct pending_item *pend;

			pend = malloc(sizeof(*pend));
			if (!pend) {
				ErrPrint("Heap: %s\n", strerror(errno));
				ret = -ENOMEM;
				packet_destroy(packet);
				free(item);
				goto out;
			}

			pend->packet = packet;
			pend->item = item;

			s_info.pending_list = dlist_append(s_info.pending_list, pend);
			DbgPrint("Request is pended\n");

			ret = 0;
		}
	}

out:
	free(handle->output);
	free(handle->layout);
	free(handle->group);
	free(handle->descfile);
	free(handle);
	return ret;
}


/* End of a file */
