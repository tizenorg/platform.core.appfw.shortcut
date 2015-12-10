/*
 * Copyright (c) 2011 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#define SHORTCUT_IS_WIDGET_SIZE(size)           (!!((size) & WIDGET_SIZE_DEFAULT))
#define SHORTCUT_IS_EASY_MODE_WIDGET_SIZE(size) (!!((size) & WIDGET_SIZE_EASY_DEFAULT))

int errno;

static inline int make_connection(void);

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
	.dbfile = DB_PATH,
	.handle = NULL,
	.initialized = 0,
	.db_opened = 0,
	.timer_id = 0,
};

static struct packet *add_shortcut_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int allow_duplicate;
	int ret = SHORTCUT_ERROR_NONE;
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
		ret = SHORTCUT_ERROR_NOT_SUPPORTED;
	}

	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("ret [%d]\n", ret);
	}

	return packet_create_reply(packet, "i", ret);
}



static struct packet *add_shortcut_widget_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *widget_id;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	double period;
	int allow_duplicate;
	int ret = SHORTCUT_ERROR_NONE;
	int sender_pid;

	if (!packet) {
		return NULL;
	}

	if (packet_get(packet, "ississdi", &sender_pid, &widget_id, &name, &type, &content, &icon, &period, &allow_duplicate) != 8) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("widget_id[%s], name[%s], type[0x%x], content[%s], icon[%s], period[%lf], allow_duplicate[%d]\n", widget_id, name, type, content, icon, period, allow_duplicate);

	if (s_info.server_cb.request_cb) {
		ret = s_info.server_cb.request_cb(widget_id, name, type, content, icon, sender_pid, period, allow_duplicate, s_info.server_cb.data);
	} else {
		ret = 0;
	}

	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("ret [%d]\n", ret);
	}

	return packet_create_reply(packet, "i", ret);
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
			.cmd = "add_shortcut_widget",
			.handler = add_shortcut_widget_handler,
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
		ErrPrint("Heap: %d\n", errno);

	return dup_pkgname;
}



EAPI int shortcut_set_request_cb(shortcut_request_cb request_cb, void *data)
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

	if (ret != SHORTCUT_ERROR_NONE) {
		DbgPrint("Packet reply [%d]\n", ret);
		if (ret == SHORTCUT_ERROR_PERMISSION_DENIED)
			ret = SHORTCUT_ERROR_NONE;
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


EAPI int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content, const char *icon, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	/*Deprecated API */
	return SHORTCUT_ERROR_NONE;
}

EAPI int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	/*Deprecated API */
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
			} else {
				return SHORTCUT_ERROR_COMM;
			}
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		if (appid) {
			free(appid);
		}
		ErrPrint("Heap: %d\n", errno);
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


EAPI int shortcut_add_to_home_widget(const char *name, shortcut_widget_size_e size, const char *widget_id, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	char *appid = NULL;
	int ret;
	int err = SHORTCUT_ERROR_NONE;

	if (name == NULL) {
		ErrPrint("AppID is null\n");
		err = SHORTCUT_ERROR_INVALID_PARAMETER;
		goto out;
	}

	if (!SHORTCUT_IS_WIDGET_SIZE(size)) {
		ErrPrint("Invalid type used for adding a widget\n");
		err = SHORTCUT_ERROR_INVALID_PARAMETER;
		goto out;
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
			err = SHORTCUT_ERROR_COMM;
			goto out;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %d\n", errno);
		err = SHORTCUT_ERROR_OUT_OF_MEMORY;
		goto out;
	}

	item->result_internal_cb = NULL;
	item->result_cb = result_cb;
	item->data = data;

	packet = packet_create("add_shortcut_widget", "ississdi", getpid(), widget_id, name, size, NULL, icon, period, allow_duplicate);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		err = SHORTCUT_ERROR_FAULT;
		goto out;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = SHORTCUT_ERROR_INVALID_PARAMETER;
		err =  SHORTCUT_ERROR_COMM;
		goto out;
	}
out:
	if (appid)
		free(appid);

	return err;
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
				ErrPrint("strdup: %d\n", errno);
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
				ErrPrint("strdup: %d\n", errno);
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
			ErrPrint("Heap: %d\n", errno);
		}
	}

	return language;
}

EAPI int shortcut_get_list(const char *package_name, shortcut_list_cb list_cb, void *data)
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

	if (list_cb == NULL) {
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

	if (package_name) {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service WHERE appid = ?";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}

		ret = sqlite3_bind_text(stmt, 1, package_name, -1, SQLITE_TRANSIENT);
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

		package_name = (const char *)sqlite3_column_text(stmt, 1);
		if (!package_name) {
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
		if (list_cb(package_name, (i18n_icon != NULL ? i18n_icon : (char *)icon), (i18n_name != NULL ? i18n_name : (char *)name), (char *)extra_key, (char *)extra_data, data) < 0) {
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
