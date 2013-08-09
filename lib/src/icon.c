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



#define CREATED	0x00BEEF00
#define DESTROYED 0x00DEAD00


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
	struct shortcut_icon *handle;
	icon_request_cb_t result_cb;
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
	char *target_id;
};



struct shortcut_icon {
	unsigned int state;
	struct shortcut_desc *desc;
	int refcnt;
	void *data;
};



struct shortcut_desc {
	int for_pd;

	unsigned int last_idx;

	struct dlist *block_list;
};



static inline void delete_block(struct block *block)
{
	DbgPrint("Release block: %p\n", block);
	free(block->type);
	free(block->part);
	free(block->data);
	free(block->option);
	free(block->id);
	free(block->target_id);
	free(block);
}



static inline int shortcut_icon_desc_close(struct shortcut_desc *handle)
{
	struct dlist *l;
	struct dlist *n;
	struct block *block;

	dlist_foreach_safe(handle->block_list, l, n, block) {
		handle->block_list = dlist_remove(handle->block_list, l);
		delete_block(block);
	}

	free(handle);
	return 0;
}



static inline struct shortcut_icon *shortcut_icon_request_unref(struct shortcut_icon *handle)
{
	handle->refcnt--;
	DbgPrint("Handle: refcnt[%d]\n", handle->refcnt);

	if (handle->refcnt == 0) {
		handle->state = DESTROYED;
		shortcut_icon_desc_close(handle->desc);
		free(handle);
		handle = NULL;
	}

	return handle;
}



static inline struct shortcut_icon *shortcut_icon_request_ref(struct shortcut_icon *handle)
{
	handle->refcnt++;
	DbgPrint("Handle: refcnt[%d]\n", handle->refcnt);
	return handle;
}



static int disconnected_cb(int handle, void *data)
{
	if (s_info.fd != handle) {
		return 0;
	}

	ErrPrint("Disconnected\n");
	s_info.fd = -1;
	s_info.init_cb = NULL;
	s_info.cbdata = NULL;
	s_info.initialized = 0;
	return 0;
}



static inline struct shortcut_desc *shortcut_icon_desc_open(void)
{
	struct shortcut_desc *handle;

	handle = calloc(1, sizeof(*handle));
	if (!handle) {
		ErrPrint("Error: %s\n", strerror(errno));
		return NULL;
	}

	return handle;
}



static inline int shortcut_icon_desc_save(struct shortcut_desc *handle, const char *filename)
{
	struct dlist *l;
	struct dlist *n;
	struct block *block;
	FILE *fp;

	if (!handle) {
		return -EINVAL;
	}

	fp = fopen(filename, "w+t");
	if (!fp) {
		ErrPrint("Error: %s\n", strerror(errno));
		return -EIO;
	}

	DbgPrint("Close and flush\n");
	dlist_foreach_safe(handle->block_list, l, n, block) {
		DbgPrint("{\n");
		fprintf(fp, "{\n");
		if (block->type) {
			fprintf(fp, "type=%s\n", block->type);
			DbgPrint("type=%s\n", block->type);
		}

		if (block->part) {
			fprintf(fp, "part=%s\n", block->part);
			DbgPrint("part=%s\n", block->part);
		}

		if (block->data) {
			fprintf(fp, "data=%s\n", block->data);
			DbgPrint("data=%s\n", block->data);
		}

		if (block->option) {
			fprintf(fp, "option=%s\n", block->option);
			DbgPrint("option=%s\n", block->option);
		}

		if (block->id) {
			fprintf(fp, "id=%s\n", block->id);
			DbgPrint("id=%s\n", block->id);
		}

		if (block->target_id) {
			fprintf(fp, "target=%s\n", block->target_id);
			DbgPrint("target=%s\n", block->target_id);
		}

		fprintf(fp, "}\n");
		DbgPrint("}\n");
	}

	if (fclose(fp) != 0) {
		ErrPrint("fclose: %s\n", strerror(errno));
	}
	return 0;
}



static inline struct block *find_block(struct shortcut_desc *handle, const char *id, const char *part)
{
	struct block *block;
	struct dlist *l;

	dlist_foreach(handle->block_list, l, block) {
		if (!strcmp(block->part, part) && (!id || !strcmp(block->id, id))) {
			return block;
		}
	}

	return NULL;
}



static inline int update_block(struct block *block, const char *data, const char *option)
{
	char *_data = NULL;
	char *_option = NULL;

	if (data) {
		_data = strdup(data);
		if (!_data) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	if (option) {
		_option = strdup(option);
		if (!_option) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	free(block->data);
	free(block->option);

	block->data = _data;
	block->option = _option;
	return 0;
}



/*!
 * \return idx
 */



static inline int shortcut_icon_desc_add_block(struct shortcut_desc *handle, const char *id, const char *type, const char *part, const char *data, const char *option, const char *target_id)
{
	struct block *block;

	if (!handle || !type) {
		return SHORTCUT_ERROR_INVALID;
	}

	if (!part) {
		part = "";
	}

	if (!data) {
		data = "";
	}

	if (target_id) {
		if (strcmp(type, SHORTCUT_ICON_TYPE_SCRIPT)) {
			ErrPrint("target id only can be used for script type\n");
			return -EINVAL;
		}
	}

	block = find_block(handle, id, part);
	if (!block) {
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

		if (target_id) {
			block->target_id = strdup(target_id);
			if (!block->target_id) {
				ErrPrint("Heap: %s\n", strerror(errno));
				free(block->id);
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
	} else {
		if (strcmp(block->type, type)) {
			ErrPrint("type is not valid (%s, %s)\n", block->type, type);
			return -EINVAL;
		}

		if ((block->target_id && !target_id) || (!block->target_id && target_id)) {
			ErrPrint("type is not valid (%s, %s)\n", block->type, type);
			return -EINVAL;
		}

		if (block->target_id && target_id && strcmp(block->target_id, target_id)) {
			ErrPrint("type is not valid (%s, %s)\n", block->type, type);
			return -EINVAL;
		}

		update_block(block, data, option);
	}

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

	if (item->result_cb) {
		item->result_cb(item->handle, ret, item->data);
	}

	(void)shortcut_icon_request_unref(item->handle);
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

		if (s_info.init_cb) {
			s_info.init_cb(ret, s_info.cbdata);
		}
	} else {
		struct dlist *l;
		struct dlist *n;
		struct pending_item *pend;

		if (s_info.init_cb) {
			s_info.init_cb(SHORTCUT_SUCCESS, s_info.cbdata);
		}

		dlist_foreach_safe(s_info.pending_list, l, n, pend) {
			s_info.pending_list = dlist_remove(s_info.pending_list, l);

			ret = com_core_packet_async_send(s_info.fd, pend->packet, 0.0f, icon_request_cb, pend->item);
			packet_destroy(pend->packet);
			if (ret < 0) {
				ErrPrint("ret: %d\n", ret);
				if (pend->item->result_cb) {
					pend->item->result_cb(pend->item->handle, ret, pend->item->data);
				}
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

	if (vconf_get_bool(VCONFKEY_MASTER_STARTED, &state) < 0) {
		ErrPrint("Unable to get \"%s\"\n", VCONFKEY_MASTER_STARTED);
	}

	if (state == 1 && make_connection() == SHORTCUT_SUCCESS) {
		int ret;
		ret = vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
		DbgPrint("Ignore VCONF [%d]\n", ret);
	}
}



EAPI int shortcut_icon_service_init(int (*init_cb)(int status, void *data), void *data)
{
	int ret;

	if (s_info.fd >= 0) {
		return -EALREADY;
	}

	if (s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	s_info.init_cb = init_cb;
	s_info.cbdata = data;

	ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL);
	if (ret < 0) {
		ErrPrint("Failed to add vconf for service state [%d]\n", ret);
	} else {
		DbgPrint("vconf is registered\n");
	}

	master_started_cb(NULL, NULL);
	return 0;
}



EAPI int shortcut_icon_service_fini(void)
{
	struct dlist *l;
	struct dlist *n;
	struct pending_item *pend;

	if (s_info.initialized) {
		com_core_del_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
		s_info.initialized = 0;
	}

	if (s_info.fd < 0) {
		return -EINVAL;
	}

	com_core_packet_client_fini(s_info.fd);
	s_info.init_cb = NULL;
	s_info.cbdata = NULL;
	s_info.fd = -1;

	dlist_foreach_safe(s_info.pending_list, l, n, pend) {
		s_info.pending_list = dlist_remove(s_info.pending_list, l);
		packet_unref(pend->packet);
		if (pend->item->result_cb) {
			pend->item->result_cb(pend->item->handle, SHORTCUT_ERROR_COMM, pend->item->data);
		}
		free(pend->item);
		free(pend);
	}
	return 0;
}



EAPI struct shortcut_icon *shortcut_icon_request_create(void)
{
	struct shortcut_icon *handle;

	handle = malloc(sizeof(*handle));
	if (!handle) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	handle->desc = shortcut_icon_desc_open();
	if (!handle->desc) {
		ErrPrint("Uanble to open desc\n");
		free(handle);
		return NULL;
	}

	handle->state = CREATED;
	handle->refcnt = 1;
	return handle;
}


EAPI int shortcut_icon_request_set_data(struct shortcut_icon *handle, void *data)
{
	if (!handle || handle->state != CREATED) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	handle->data = data;
	return 0;
}



EAPI void *shortcut_icon_request_data(struct shortcut_icon *handle)
{
	if (!handle || handle->state != CREATED) {
		ErrPrint("Handle is not valid\n");
		return NULL;
	}

	return handle->data;
}



EAPI int shortcut_icon_request_set_info(struct shortcut_icon *handle, const char *id, const char *type, const char *part, const char *data, const char *option, const char *subid)
{
	if (!handle || handle->state != CREATED) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	return shortcut_icon_desc_add_block(handle->desc, id, type, part, data, option, subid);
}



EAPI int shortcut_icon_request_destroy(struct shortcut_icon *handle)
{
	if (!handle || handle->state != CREATED) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	(void)shortcut_icon_request_unref(handle);
	return 0;
}



EAPI int shortcut_icon_request_send(struct shortcut_icon *handle, int size_type, const char *layout, const char *group, const char *outfile, icon_request_cb_t result_cb, void *data)
{
	int ret;
	struct packet *packet;
	struct request_item *item;
	char *filename;
	int len;

	if (!handle || handle->state != CREATED) {
		ErrPrint("Handle is not valid\n");
		return -EINVAL;
	}

	if (!layout) {
		layout = DEFAULT_ICON_LAYOUT;
	}

	if (!group) {
		group = DEFAULT_ICON_GROUP;
	}

	len = strlen(outfile) + strlen(".desc") + 1;
	filename = malloc(len);
	if (!filename) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	snprintf(filename, len, "%s.desc", outfile);

	ret = shortcut_icon_desc_save(handle->desc, filename);
	if (ret < 0) {
		goto out;
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		if (unlink(filename) < 0) {
			ErrPrint("Unlink: %s\n", strerror(errno));
		}
		ret = -ENOMEM;
		goto out;
	}

	item->result_cb = result_cb;
	item->data = data;
	item->handle = shortcut_icon_request_ref(handle);

	packet = packet_create("icon_create", "sssis", layout, group, filename, size_type, outfile);
	if (!packet) {
		ErrPrint("Failed to create a packet\n");
		if (unlink(filename) < 0) {
			ErrPrint("Unlink: %s\n", strerror(errno));
		}
		free(item);
		(void)shortcut_icon_request_unref(handle);
		ret = -EFAULT;
		goto out;
	}

	if (s_info.fd >= 0 && !s_info.pending_list) {
		ret = com_core_packet_async_send(s_info.fd, packet, 0.0f, icon_request_cb, item);
		packet_destroy(packet);
		if (ret < 0) {
			ErrPrint("ret: %d\n", ret);
			if (unlink(filename) < 0) {
				ErrPrint("Unlink: %s\n", strerror(errno));
			}
			free(item);
			(void)shortcut_icon_request_unref(handle);
		}
		DbgPrint("Request is sent\n");
	} else {
		struct pending_item *pend;

		pend = malloc(sizeof(*pend));
		if (!pend) {
			ErrPrint("Heap: %s\n", strerror(errno));
			packet_destroy(packet);
			free(item);
			if (unlink(filename) < 0) {
				ErrPrint("Unlink: %s\n", strerror(errno));
			}
			(void)shortcut_icon_request_unref(handle);
			ret = -ENOMEM;
			goto out;
		}

		pend->packet = packet;
		pend->item = item;

		s_info.pending_list = dlist_append(s_info.pending_list, pend);
		DbgPrint("Request is pended\n");

		ret = 0;
	}

out:
	free(filename);
	return ret;
}


/* End of a file */
