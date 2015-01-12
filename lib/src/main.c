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

#include <aul.h>
#include <dlog.h>
#include <glib.h>
#include <db-util.h>
#include <vconf.h>
#include <vconf-keys.h>

#include <packet.h>
#include <com-core.h>
#include <com-core_packet.h>

#include "shortcut.h"
#include "shortcut_private.h"
#include "shortcut_manager.h"

#define SHORTCUT_PKGNAME_LEN 512

int errno;

static struct info {
	const char *dbfile;
	sqlite3 *handle;
	int server_fd;
	int client_fd;
	const char *socket_file;
	struct {
		int (*request_cb)(const char *appid, const char *name, int type, const char *content, const char *icon, pid_t pid, double period, int allow_duplicate, void *data);
		void *data;
	} server_cb;
	int initialized;
	int db_opened;
	guint timer_id;
} s_info = {
	.server_fd = -1,
	.client_fd = -1,
	.socket_file = "/tmp/.shortcut.service",
	.dbfile = "/opt/dbspace/.shortcut_service.db",
	.handle = NULL,
	.initialized = 0,
	.db_opened = 0,
	.timer_id = 0,
};


static inline int make_connection(void);


static struct packet *remove_shortcut_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	const char *content_info;
	int ret;
	int sender_pid;

	if (!packet) {
		ErrPrint("Packet is NIL, maybe disconnected?\n");
		return NULL;
	}

	if (packet_get(packet, "isss", &sender_pid, &appid, &name, &content_info) != 4) {
		ErrPrint("Invalid apcket\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s], content_info[%s]\n", appid, name, content_info);

	if (s_info.server_cb.request_cb) {
		ret = s_info.server_cb.request_cb(appid, name, SHORTCUT_REMOVE, content_info, NULL, sender_pid, -1.0f, 0, s_info.server_cb.data);
	} else {
		ret = SHORTCUT_ERROR_UNSUPPORTED;
	}

	return packet_create_reply(packet, "i", ret);
}



static struct packet *remove_dynamicbox_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int ret;
	int sender_pid;

	if (!packet) {
		ErrPrint("PAcket is NIL, maybe disconnected?\n");
		return NULL;
	}

	if (packet_get(packet, "iss", &sender_pid, &appid, &name) != 3) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s]\n", appid, name);

	if (s_info.server_cb.request_cb) {
		ret = s_info.server_cb.request_cb(appid, name, DYNAMICBOX_REMOVE, NULL, NULL, sender_pid, -1.0f, 0, s_info.server_cb.data);
	} else {
		ret = SHORTCUT_ERROR_UNSUPPORTED;
	}

	return packet_create_reply(packet, "i", ret);
}



static struct packet *add_shortcut_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int allow_duplicate;
	int ret;
	int sender_pid;

	if (!packet) {
		return NULL;
	}

	if (packet_get(packet, "ississi", &sender_pid, &appid, &name, &type, &content, &icon, &allow_duplicate) != 7) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s], type[0x%x], content[%s], icon[%s] allow_duplicate[%d]\n", appid, name, type, content, icon, allow_duplicate);

	if (s_info.server_cb.request_cb) {
		ret = s_info.server_cb.request_cb(appid, name, type, content, icon, sender_pid, -1.0f, allow_duplicate, s_info.server_cb.data);
	} else {
		ret = SHORTCUT_ERROR_UNSUPPORTED;
	}

	return packet_create_reply(packet, "i", ret);
}



static struct packet *add_dynamicbox_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	double period;
	int allow_duplicate;
	int ret;
	int sender_pid;

	if (!packet) {
		return NULL;
	}

	if (packet_get(packet, "ississdi", &sender_pid, &appid, &name, &type, &content, &icon, &period, &allow_duplicate) != 8) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s], type[0x%x], content[%s], icon[%s], period[%lf], allow_duplicate[%d]\n", appid, name, type, content, icon, period, allow_duplicate);

	if (s_info.server_cb.request_cb) {
		ret = s_info.server_cb.request_cb(appid, name, type, content, icon, sender_pid, period, allow_duplicate, s_info.server_cb.data);
	} else {
		ret = 0;
	}

	return packet_create_reply(packet, "i", ret);
}



static void master_started_cb(keynode_t *node, void *user_data)
{
	int state = 0;

	if (vconf_get_bool(VCONFKEY_MASTER_STARTED, &state) < 0) {
		ErrPrint("Unable to get \"%s\"\n", VCONFKEY_MASTER_STARTED);
	}

	if (state == 1 && make_connection() == SHORTCUT_ERROR_NONE) {
		(void)vconf_ignore_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb);
	}
}



int shortcut_is_master_ready(void)
{
	int ret = -1, is_master_started = 0;

	ret = vconf_get_bool(VCONFKEY_MASTER_STARTED, &is_master_started);
	if (ret == 0 && is_master_started == 1) {
		ErrPrint("the master has been started");
	} else {
		is_master_started = 0;
		ErrPrint("the master has been stopped");
	}

	return is_master_started;
}



static gboolean timeout_cb(void *data)
{
	int ret;

	ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL);
	if (ret < 0) {
		ErrPrint("Failed to add vconf for service state [%d]\n", ret);
	} else {
		DbgPrint("vconf is registered\n");
	}

	master_started_cb(NULL, NULL);

	s_info.timer_id = 0;
	return FALSE;
}



static int disconnected_cb(int handle, void *data)
{
	if (s_info.client_fd == handle) {
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return 0;
	}

	if (s_info.server_fd == handle) {
		if (!s_info.timer_id) {
			s_info.server_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
			s_info.timer_id = g_timeout_add(1000, timeout_cb, NULL);
			if (!s_info.timer_id) {
				ErrPrint("Unable to add timer\n");
			}
		}
		return 0;
	}

	return 0;
}



static inline int make_connection(void)
{
	int ret;
	struct packet *packet;
	static struct method service_table[] = {
		{
			.cmd = "add_shortcut",
			.handler = add_shortcut_handler,
		},
		{
			.cmd = "add_dynamicbox",
			.handler = add_dynamicbox_handler,
		},
		{
			.cmd = "rm_shortcut",
			.handler = remove_shortcut_handler,
		},
		{
			.cmd = "rm_dynamicbox",
			.handler = remove_dynamicbox_handler,
		},
		{
			.cmd = NULL,
			.handler = NULL,
		},
	};

	if (s_info.initialized == 0) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	s_info.server_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
	if (s_info.server_fd < 0) {
		ErrPrint("Failed to make a connection to the master\n");
		return SHORTCUT_ERROR_COMM;
	}

	packet = packet_create_noack("service_register", "");
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_send_only(s_info.server_fd, packet);
	DbgPrint("Service register sent: %d\n", ret);
	packet_destroy(packet);
	if (ret != 0) {
		com_core_packet_client_fini(s_info.server_fd);
		s_info.server_fd = -1;
		ret = SHORTCUT_ERROR_COMM;
	} else {
		ret = SHORTCUT_ERROR_NONE;
	}

	DbgPrint("Server FD: %d\n", s_info.server_fd);
	return ret;
}



static char *_shortcut_get_pkgname_by_pid(void)
{
	char pkgname[SHORTCUT_PKGNAME_LEN + 1] = { 0, };
	int pid = 0, ret = 0;
	int fd;
	char  *dup_pkgname;

	pid = getpid();

	ret = aul_app_get_pkgname_bypid(pid, pkgname, sizeof(pkgname));
	if (ret != 0) {
		char buf[SHORTCUT_PKGNAME_LEN + 1] = { 0, };

		snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
			return NULL;
		}

		ret = read(fd, pkgname, sizeof(pkgname) - 1);
		close(fd);

		if (ret <= 0) {
			return NULL;
		}

		pkgname[ret] = '\0';
		/*!
		 * \NOTE
		 * "ret" is not able to be larger than "sizeof(pkgname) - 1",
		 * if the system is not going wrong.
		 */
	} else {
		if (strlen(pkgname) <= 0) {
			return NULL;
		}
	}

	dup_pkgname = strdup(pkgname);
	if (!dup_pkgname)
		ErrPrint("Heap: %s\n", strerror(errno));

	return dup_pkgname;
}



EAPI int shortcut_set_request_cb(request_cb_t request_cb, void *data)
{
	if (request_cb == NULL) {
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	s_info.server_cb.request_cb = request_cb;
	s_info.server_cb.data = data;

	if (s_info.server_fd < 0) {
		int ret;

		ret = vconf_notify_key_changed(VCONFKEY_MASTER_STARTED, master_started_cb, NULL);
		if (ret < 0) {
			ErrPrint("Failed to add vconf for service state [%d]\n", ret);
			return SHORTCUT_ERROR_COMM;
		} else {
			DbgPrint("vconf is registered\n");
		}

		master_started_cb(NULL, NULL);
	}

	return SHORTCUT_ERROR_NONE;
}



struct result_cb_item {
	result_internal_cb_t result_internal_cb;
	result_cb_t result_cb;
	void *data;
};



static int shortcut_send_cb(pid_t pid, int handle, const struct packet *packet, void *data)
{
	struct result_cb_item *item = data;
	int ret;

	if (!packet) {
		ErrPrint("Packet is not valid\n");
		ret = SHORTCUT_ERROR_FAULT;
	} else if (packet_get(packet, "i", &ret) != 1) {
		ErrPrint("Packet is not valid\n");
		ret = SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (item->result_internal_cb) {
		ret = item->result_internal_cb(ret, pid, item->data);
	} else if (item->result_cb) {
		ret = item->result_cb(ret, item->data);
	} else {
		ret = SHORTCUT_ERROR_NONE;
	}
	free(item);
	return ret;
}



EAPI int add_to_home_remove_shortcut(const char *appid, const char *name, const char *content_info, result_internal_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

	if (!appid || !name) {
		ErrPrint("Invalid argument\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	if (s_info.client_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};

		s_info.client_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
		if (s_info.client_fd < 0) {
			ErrPrint("Failed to make connection\n");
			return SHORTCUT_ERROR_COMM;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_internal_cb = result_cb;
	item->result_cb = NULL;
	item->data = data;

	packet = packet_create("rm_shortcut", "isss", getpid(), appid, name, content_info);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return SHORTCUT_ERROR_COMM;
	}

	return SHORTCUT_ERROR_NONE;
}



EAPI int add_to_home_remove_dynamicbox(const char *appid, const char *name, result_internal_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

	if (!appid || !name) {
		ErrPrint("Invalid argument\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	if (s_info.client_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};


		s_info.client_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
		if (s_info.client_fd < 0) {
			ErrPrint("Failed to make connection\n");
			return SHORTCUT_ERROR_COMM;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_internal_cb = result_cb;
	item->result_cb = NULL;
	item->data = data;

	packet = packet_create("rm_dynamicbox", "iss", getpid(), appid, name);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return SHORTCUT_ERROR_COMM;
	}

	return SHORTCUT_ERROR_NONE;
}



EAPI int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content, const char *icon, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

	if (!appid || ADD_TO_HOME_IS_DYNAMICBOX(type)) {
		ErrPrint("Invalid type used for adding a shortcut\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	if (s_info.client_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};

		s_info.client_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
		if (s_info.client_fd < 0) {
			ErrPrint("Failed to make connection\n");
			return SHORTCUT_ERROR_COMM;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_internal_cb = result_cb;
	item->result_cb = NULL;
	item->data = data;

	if (!name) {
		name = "";
	}

	if (!content) {
		content = "";
	}

	if (!icon) {
		icon = "";
	}

	packet = packet_create("add_shortcut", "ississi", getpid(), appid, name, type, content, icon, allow_duplicate);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return SHORTCUT_ERROR_COMM;
	}

	return SHORTCUT_ERROR_NONE;
}

EAPI int shortcut_add_to_home(const char *name, shortcut_type type, const char *uri, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	char *appid = NULL;
	int ret;

	if (ADD_TO_HOME_IS_DYNAMICBOX(type)) {
		ErrPrint("Invalid type used for adding a shortcut\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	appid = _shortcut_get_pkgname_by_pid();

	if (!s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	if (s_info.client_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};

		s_info.client_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
		if (s_info.client_fd < 0) {
			if (appid) {
				free(appid);
			}
			if (shortcut_is_master_ready() == 1) {
				return SHORTCUT_ERROR_PERMISSION_DENIED;
			}
			else {
				return SHORTCUT_ERROR_COMM;
			}
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		if (appid) {
			free(appid);
		}
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_cb = result_cb;
	item->result_internal_cb = NULL;
	item->data = data;

	if (!name) {
		name = "";
	}

	if (!uri) {
		uri = "";
	}

	if (!icon) {
		icon = "";
	}

	packet = packet_create("add_shortcut", "ississi", getpid(), appid, name, type, uri, icon, allow_duplicate);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		if (appid) {
			free(appid);
		}
		if (item) {
			free(item);
		}
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	packet_destroy(packet);
	if (ret < 0) {
		if (item) {
			free(item);
		}
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return SHORTCUT_ERROR_COMM;
	}

	return SHORTCUT_ERROR_NONE;
}


EAPI int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

	if (!appid || !ADD_TO_HOME_IS_DYNAMICBOX(type)) {
		ErrPrint("Invalid type is used for adding a dynamicbox\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!s_info.initialized) {
		s_info.initialized = 1;
		com_core_add_event_callback(CONNECTOR_DISCONNECTED, disconnected_cb, NULL);
	}

	if (s_info.client_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};

		s_info.client_fd = com_core_packet_client_init(s_info.socket_file, 0, service_table);
		if (s_info.client_fd < 0) {
			return SHORTCUT_ERROR_COMM;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_internal_cb = result_cb;
	item->result_cb = NULL;
	item->data = data;

	packet = packet_create("add_dynamicbox", "ississdi", getpid(), appid, name, type, content, icon, period, allow_duplicate);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return SHORTCUT_ERROR_FAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		return SHORTCUT_ERROR_COMM;
	}

	return SHORTCUT_ERROR_NONE;
}



static inline int open_db(void)
{
	int ret;

	ret = db_util_open(s_info.dbfile, &s_info.handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		DbgPrint("Failed to open a %s\n", s_info.dbfile);
		return SHORTCUT_ERROR_IO_ERROR;
	}

	return SHORTCUT_ERROR_NONE;
}



/*!
 * \note this function will returns allocated(heap) string
 */
static inline int get_i18n_name(const char *lang, int id, char **name, char **icon)
{
	sqlite3_stmt *stmt;
	static const char *query = "SELECT name, icon FROM shortcut_name WHERE id = ? AND lang = ? COLLATE NOCASE";
	const unsigned char *_name;
	const unsigned char *_icon;
	int ret = 0;
	int status;

	status = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to prepare stmt: %s\n", sqlite3_errmsg(s_info.handle));
		return -EFAULT;
	}

	status = sqlite3_bind_int(stmt, 1, id);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind id: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EFAULT;
		goto out;
	}

	status = sqlite3_bind_text(stmt, 2, lang, -1, SQLITE_TRANSIENT);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind lang: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EFAULT;
		goto out;
	}

	DbgPrint("id: %d, lang: %s\n", id, lang);
	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Failed to do step: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -ENOENT;
		goto out;
	}

	_name = sqlite3_column_text(stmt, 0);
	if (name) {
		if (_name && strlen((const char *)_name)) {
			*name = strdup((const char *)_name);
			if (!*name) {
				ErrPrint("strdup: %s\n", strerror(errno));
				ret = -ENOMEM;
				goto out;
			}
		} else {
			*name = NULL;
		}
	}

	_icon = sqlite3_column_text(stmt, 1);
	if (icon) {
		if (_icon && strlen((const char *)_icon)) {
			*icon = strdup((const char *)_icon);
			if (!*icon) {
				ErrPrint("strdup: %s\n", strerror(errno));
				ret = -ENOMEM;
				if (name && *name) {
					free(*name);
				}
				goto out;
			}
		} else {
			*icon = NULL;
		}
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}



static inline char *cur_locale(void)
{
	char *language;
	language = vconf_get_str(VCONFKEY_LANGSET);
	if (language) {
		char *ptr;

		ptr = language;
		while (*ptr) {
			if (*ptr == '.') {
				*ptr = '\0';
				break;
			}

			if (*ptr == '_') {
				*ptr = '-';
			}

			ptr++;
		}
	} else {
		language = strdup("en-us");
		if (!language) {
			ErrPrint("Heap: %s\n", strerror(errno));
		}
	}

	return language;
}



/*!
 * \note READ ONLY DB
 */
EAPI int shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data)
{
	sqlite3_stmt *stmt;
	const char *query;
	const unsigned char *name;
	char *i18n_name = NULL;
	char *i18n_icon = NULL;
	const unsigned char *extra_data;
	const unsigned char *extra_key;
	const unsigned char *icon;
	int id;
	int ret;
	int cnt;
	char *language;

	if (cb == NULL) {
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!s_info.db_opened) {
		s_info.db_opened = (open_db() == 0);
	}

	if (!s_info.db_opened) {
		ErrPrint("Failed to open a DB\n");
		return SHORTCUT_ERROR_IO_ERROR;
	}

	language = cur_locale();
	if (!language) {
		ErrPrint("Locale is not valid\n");
		return SHORTCUT_ERROR_FAULT;
	}

	if (appid) {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service WHERE appid = ?";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}

		ret = sqlite3_bind_text(stmt, 1, appid, -1, SQLITE_TRANSIENT);
		if (ret != SQLITE_OK) {
			ErrPrint("bind text: %s\n", sqlite3_errmsg(s_info.handle));
			sqlite3_finalize(stmt);
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}
	} else {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}
	}

	cnt = 0;
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		id = sqlite3_column_int(stmt, 0);

		appid = (const char *)sqlite3_column_text(stmt, 1);
		if (!appid) {
			LOGE("Failed to get package name\n");
			continue;
		}

		name = sqlite3_column_text(stmt, 2);
		if (!name) {
			LOGE("Failed to get name\n");
			continue;
		}

		extra_key = sqlite3_column_text(stmt, 3);
		if (!extra_key) {
			LOGE("Failed to get service\n");
			continue;
		}

		extra_data = sqlite3_column_text(stmt, 4);
		if (!extra_data) {
			LOGE("Failed to get service\n");
			continue;
		}

		icon = sqlite3_column_text(stmt, 5);
		if (!icon) {
			LOGE("Failed to get icon\n");
			continue;
		}

		/*!
		 * \todo
		 * Implement the "GET LOCALE" code
		 */
		if (get_i18n_name(language, id, &i18n_name, &i18n_icon) < 0) {
			/* Okay, we can't manage this. just use the fallback string */
		}

		cnt++;
		if (cb(appid, (i18n_icon != NULL ? i18n_icon : (char *)icon), (i18n_name != NULL ? i18n_name : (char *)name), (char *)extra_key, (char *)extra_data, data) < 0) {
			free(i18n_name);
			break;
		}

		free(i18n_name);
		i18n_name = NULL;

		free(i18n_icon);
		i18n_icon = NULL;
	}

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	free(language);
	return cnt;
}

/* End of a file */
