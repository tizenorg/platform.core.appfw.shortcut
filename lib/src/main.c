/*
 * libshortcut
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>, Youngjoo Park <yjoo93.park@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software, either express or implied, including but not limited to the implied warranties of merchantability, fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using, modifying or distributing this software or its derivatives.
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
	s_info.client_fd = -EINVAL;
	return 0;
}



EAPI int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content, const char *icon, result_cb_t result_cb, void *data)
{
	int ret;
	struct packet *packet;
	struct result_cb_item *item;

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

	return com_core_packet_async_send(s_info.client_fd, packet, shortcut_send_cb, item);
}



EAPI int add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, result_cb_t result_cb, void *data)
{
	int ret;
	struct packet *packet;
	struct result_cb_item *item;

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

	return com_core_packet_async_send(s_info.client_fd, packet, livebox_send_cb, item);
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
	const char *query;
	const char *name;
	char *ret;
	int status;

	query = "SELECT name FROM shortcut_name WHERE id = ? AND lang = ?";
	status = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to prepare stmt\n");
		return NULL;
	}

	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Failed to do step\n");
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		sqlite3_finalize(stmt);
		return NULL;
	}

	name = sqlite3_column_text(stmt, 0);
	ret = name ? strdup(name) : NULL;

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
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

	if (!s_info.db_opened)
		s_info.db_opened = (open_db() == 0);

	if (!s_info.db_opened) {
		ErrPrint("Failed to open a DB\n");
		return -EIO;
	}

	if (appid) {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service WHERE appid = ?";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			return -EIO;
		}

		ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("bind text: %s\n", sqlite3_errmsg(s_info.handle));
			sqlite3_finalize(stmt);
			return -EIO;
		}
	} else {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service";
		ret = sqlite3_prepare_v2(s_info.handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(s_info.handle));
			return -EIO;
		}
	}

	cnt = 0;
	while (SQLITE_ROW == sqlite3_step(stmt)) {
		id = sqlite3_column_int(stmt, 0);

		appid = sqlite3_column_text(stmt, 1);
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
		i18n_name = get_i18n_name("en-us", id);

		cnt++;
		if (cb(appid, icon, (i18n_name != NULL ? i18n_name : (char *)name), extra_key, extra_data, data) < 0) {
			free(i18n_name);
			break;
		}

		free(i18n_name);
	}

	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return cnt;
}



/* End of a file */
