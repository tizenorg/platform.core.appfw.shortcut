/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#include "shortcut.h"

#if !defined(FLOG)
#define DbgPrint(format, arg...)	LOGD("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#define ErrPrint(format, arg...)	LOGE("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#else
extern FILE *__file_log_fp;
#define DbgPrint(format, arg...) do { fprintf(__file_log_fp, "[LOG] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)

#define ErrPrint(format, arg...) do { fprintf(__file_log_fp, "[ERR] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#endif

#define EAPI __attribute__((visibility("default")))

int errno;



static struct info {
	const char *dbfile;
	sqlite3 *handle;
	int server_fd;
	int client_fd;
	const char *socket_file;
	struct {
		int (*request_cb)(const char *appid, const char *name, int type, const char *content, const char *icon, pid_t pid, double period, void *data);
		void *data;
	} server_cb;
	int initialized;
	int db_opened;
} s_info = {
	.server_fd = -1,
	.client_fd = -1,
	.socket_file = "/tmp/.shortcut",
	.dbfile = "/opt/dbspace/.shortcut_service.db",
	.handle = NULL,
	.initialized = 0,
	.db_opened = 0,
};



static struct packet *add_shortcut_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int ret;

	if (!packet)
		return NULL;

	if (packet_get(packet, "ssiss", &appid, &name, &type, &content, &icon) != 5) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s], type[0x%x], content[%s], icon[%s]\n", appid, name, type, content, icon);

	if (s_info.server_cb.request_cb)
		ret = s_info.server_cb.request_cb(appid, name, type, content, icon, pid, -1.0f, s_info.server_cb.data);
	else
		ret = 0;

	return packet_create_reply(packet, "i", ret);
}



static struct packet *add_livebox_handler(pid_t pid, int handle, const struct packet *packet)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	double period;
	int ret;

	if (!packet)
		return NULL;

	if (packet_get(packet, "ssissd", &appid, &name, &type, &content, &icon, &period) != 6) {
		ErrPrint("Invalid packet\n");
		return NULL;
	}

	DbgPrint("appid[%s], name[%s], type[0x%x], content[%s], icon[%s], period[%lf]\n", appid, name, type, content, icon, period);

	if (s_info.server_cb.request_cb)
		ret = s_info.server_cb.request_cb(appid, name, type, content, icon, pid, period, s_info.server_cb.data);
	else
		ret = 0;

	return packet_create_reply(packet, "i", ret);
}



EAPI int shortcut_set_request_cb(request_cb_t request_cb, void *data)
{
	s_info.server_cb.request_cb = request_cb;
	s_info.server_cb.data = data;

	if (s_info.server_fd < 0) {
		static struct method service_table[] = {
			{
				.cmd = "add_shortcut",
				.handler = add_shortcut_handler,
			},
			{
				.cmd = "add_livebox",
				.handler = add_livebox_handler,
			},
			{
				.cmd = NULL,
				.handler = NULL,
			},
		};

		unlink(s_info.socket_file);	/* Delete previous socket file for creating a new server socket */
		s_info.server_fd = com_core_packet_server_init(s_info.socket_file, service_table);
	}

	DbgPrint("Server FD: %d\n", s_info.server_fd);

	return s_info.server_fd > 0 ? 0 : s_info.server_fd;
}



struct result_cb_item {
	result_cb_t result_cb;
	void *data;
};



static int shortcut_send_cb(pid_t pid, int handle, const struct packet *packet, void *data)
{
	struct result_cb_item *item = data;
	int ret;

	if (!packet) {
		ErrPrint("Packet is not valid\n");
		ret = -EFAULT;
	} else if (packet_get(packet, "i", &ret) != 1) {
		ErrPrint("Packet is not valid\n");
		ret = -EINVAL;
	}

	if (item->result_cb)
		ret = item->result_cb(ret, pid, item->data);
	else
		ret = 0;
	free(item);
	return ret;
}



static int livebox_send_cb(pid_t pid, int handle, const struct packet *packet, void *data)
{
	struct result_cb_item *item = data;
	int ret;

	if (!packet) {
		ErrPrint("Packet is not valid\n");
		ret = -EFAULT;
	} else if (packet_get(packet, "i", &ret) != 1) {
		ErrPrint("Packet is not valid\n");
		ret = -EINVAL;
	}

	if (item->result_cb)
		ret = item->result_cb(ret, pid, item->data);
	else
		ret = 0;
	free(item);
	return ret;
}



static int disconnected_cb(int handle, void *data)
{
	if (s_info.client_fd != handle) {
		/*!< This is not my favor */
		return 0;
	}

	s_info.client_fd = -EINVAL;
	return 0;
}



EAPI int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content, const char *icon, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

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
			return s_info.client_fd;
		}
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	item->result_cb = result_cb;
	item->data = data;

	if (!appid)
		appid = "";

	if (!name)
		name = "";

	if (!content)
		content = "";

	if (!icon)
		icon = "";

	packet = packet_create("add_shortcut", "ssiss", appid, name, type, content, icon);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return -EFAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, shortcut_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = -1;
	}

	return ret;
}



EAPI int add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, result_cb_t result_cb, void *data)
{
	struct packet *packet;
	struct result_cb_item *item;
	int ret;

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
		if (s_info.client_fd < 0)
			return s_info.client_fd;
	}

	item = malloc(sizeof(*item));
	if (!item) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	item->result_cb = result_cb;
	item->data = data;

	packet = packet_create("add_livebox", "ssissd", appid, name, type, content, icon, period);
	if (!packet) {
		ErrPrint("Failed to build a packet\n");
		free(item);
		return -EFAULT;
	}

	ret = com_core_packet_async_send(s_info.client_fd, packet, 0.0f, livebox_send_cb, item);
	if (ret < 0) {
		packet_destroy(packet);
		free(item);
		com_core_packet_client_fini(s_info.client_fd);
		s_info.client_fd = -1;
	}

	return ret;
}


EAPI int shortcut_add_to_home_with_period(const char *appid, const char *name, int type, const char *content, const char *icon, double period, result_cb_t result_cb, void *data)
{
	return add_to_home_livebox(appid, name, type, content, icon, period, result_cb, data);
}

EAPI int shortcut_add_to_home(const char *appid, const char *name, int type, const char *content, const char *icon, result_cb_t result_cb, void *data)
{
	return add_to_home_shortcut(appid, name, type, content, icon, result_cb, data);
}

static inline int open_db(void)
{
	int ret;

	ret = db_util_open(s_info.dbfile, &s_info.handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		DbgPrint("Failed to open a %s\n", s_info.dbfile);
		return -EIO;
	}

	return 0;
}



/*!
 * \note this function will returns allocated(heap) string
 */
static inline char *get_i18n_name(const char *lang, int id)
{
	sqlite3_stmt *stmt;
	static const char *query = "SELECT name FROM shortcut_name WHERE id = ? AND lang = ? COLLATE NOCASE";
	const unsigned char *name;
	char *ret;
	int status;

	status = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to prepare stmt: %s\n", sqlite3_errmsg(s_info.handle));
		return NULL;
	}

	status = sqlite3_bind_int(stmt, 1, id);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind id: %s\n", sqlite3_errmsg(s_info.handle));
		ret = NULL;
		goto out;
	}

	status = sqlite3_bind_text(stmt, 2, lang, -1, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind lang: %s\n", sqlite3_errmsg(s_info.handle));
		ret = NULL;
		goto out;
	}

	DbgPrint("id: %d, lang: %s\n", id, lang);
	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Failed to do step: %s\n", sqlite3_errmsg(s_info.handle));
		ret = NULL;
		goto out;
	}

	name = sqlite3_column_text(stmt, 0);
	ret = name ? strdup((const char *)name) : NULL;

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}



static inline int homescreen_get_i18n(const char *appid, const char *lang, char **name, char **desc)
{
	sqlite3_stmt *stmt;
	static const char *query = "SELECT name, desc FROM desc WHERE appid = ? AND lang = ?";
	const unsigned char *_name;
	const unsigned char *_desc;
	int status;

	status = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to prepare stmt: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	status = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind appid: %s\n", sqlite3_errmsg(s_info.handle));
		status = -EIO;
		goto out;
	}

	status = sqlite3_bind_text(stmt, 2, lang, -1, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind lang: %s\n", sqlite3_errmsg(s_info.handle));
		status = -EIO;
		goto out;
	}

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Failed to do step: %s\n", sqlite3_errmsg(s_info.handle));
		status = -EIO;
		goto out;
	}

	if (name) {
		_name = sqlite3_column_text(stmt, 0);
		*name = _name ? strdup((const char *)_name) : NULL;
	}

	if (desc) {
		_desc = sqlite3_column_text(stmt, 1);
		*desc = _desc ? strdup((const char *)_desc) : NULL;
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return status;
}



/*!
 * cb: SYNC callback
 */
EAPI int homescreen_get_description(const char *appid, void (*cb)(const char *appid, const char *icon, const char *name, const char *desc, void *data), void *data)
{
	sqlite3_stmt *stmt;
	static const char *query = "SELECT icon, name, desc FROM homescreen WHERE appid = ?";
	char *i18n_name;
	char *i18n_desc;
	const unsigned char *desc;
	const unsigned char *name;
	const unsigned char *icon;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Prepare failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Prepare failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Step failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	icon = sqlite3_column_text(stmt, 0);
	name = sqlite3_column_text(stmt, 1);
	desc = sqlite3_column_text(stmt, 2);

	/*!
	 * \todo
	 * Get the i18n name and desc
	 */
	if (homescreen_get_i18n(appid, "en-us", &i18n_name, &i18n_desc) < 0) {
		i18n_name = NULL;
		i18n_desc = NULL;
	}

	cb(appid, (const char *)icon, i18n_name ? i18n_name : (const char *)name, i18n_desc ? i18n_desc : (const char *)desc, data);

	free(i18n_name);
	free(i18n_desc);

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}



EAPI char *homescreen_get_image(const char *appid, int idx)
{
	static const char *query = "SELECT path FROM image WHERE appid = ? AND id = ?";
	sqlite3_stmt *stmt;
	int ret;
	const unsigned char *path;
	char *ret_path = NULL;

	ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Prepare failed: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("bind failed: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_bind_int(stmt, 2, idx);
	if (ret != SQLITE_OK) {
		ErrPrint("bind failed: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Step failed: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	path = sqlite3_column_text(stmt, 0);
	if (!path) {
		ErrPrint("Get result: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret_path = strdup((const char *)path);
	if (!ret_path)
		ErrPrint("Heap: %s\n", strerror(errno));

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret_path;
}



EAPI int homescreen_get_image_count(const char *appid)
{
	static const char *query = "SELECT COUNT(id) FROM image WHERE appid = ?";
	sqlite3_stmt *stmt;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("bind failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("bind failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("step failed: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_column_int(stmt, 0);

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

			if (*ptr == '_')
				*ptr = '-';

			ptr++;
		}
	} else {
		language = strdup("en-us");
		if (!language)
			ErrPrint("Heap: %s\n", strerror(errno));
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
	char *i18n_name;
	const unsigned char *extra_data;
	const unsigned char *extra_key;
	const unsigned char *icon;
	int id;
	int ret;
	int cnt;
	char *language;

	if (!s_info.db_opened)
		s_info.db_opened = (open_db() == 0);

	if (!s_info.db_opened) {
		ErrPrint("Failed to open a DB\n");
		return -EIO;
	}

	language = cur_locale();
	if (!language) {
		ErrPrint("Locale is not valid\n");
		return -EINVAL;
	}

	if (appid) {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service WHERE appid = ?";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			free(language);
			return -EIO;
		}

		ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("bind text: %s\n", sqlite3_errmsg(s_info.handle));
			sqlite3_finalize(stmt);
			free(language);
			return -EIO;
		}
	} else {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			free(language);
			return -EIO;
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
		i18n_name = get_i18n_name(language, id);

		cnt++;
		if (cb(appid, (char *)icon, (i18n_name != NULL ? i18n_name : (char *)name), (char *)extra_key, (char *)extra_data, data) < 0) {
			free(i18n_name);
			break;
		}

		free(i18n_name);
	}

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	free(language);
	return cnt;
}



/* End of a file */
