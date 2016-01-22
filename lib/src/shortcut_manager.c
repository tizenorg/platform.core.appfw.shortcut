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
#include <openssl/md5.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <aul.h>
#include <dlog.h>
#include <glib.h>
#include <db-util.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "shortcut.h"
#include "shortcut_private.h"
#include "shortcut_manager.h"

#define SHORTCUT_PKGNAME_LEN 512
#define SHORTCUT_IS_WIDGET_SIZE(size)           (!!((size) & WIDGET_SIZE_DEFAULT))

#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_SHORTCUT_INTERFACE_NAME "org.tizen.data_provider_shortcut_service"

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"

static char *_bus_name = NULL;
static GDBusConnection *_gdbus_conn = NULL;
static int monitor_id = 0;
static int provider_monitor_id = 0;

static const char *dbfile = DB_PATH;
static sqlite3 *handle = NULL;
static int db_opened = 0;

struct result_cb_item {
	result_internal_cb_t result_internal_cb;
	result_cb_t result_cb;
	void *data;
};

typedef struct _shortcut_cb_info {
	int (*request_cb)(const char *appid, const char *name, int type, const char *content, const char *icon, pid_t pid, double period, int allow_duplicate, void *data);
	void *data;
} shortcut_cb_info;

static shortcut_cb_info _callback_info;
static int shortcut_dbus_init();
static char *_shortcut_get_pkgname_by_pid(void);

static void _add_shortcut_notify(GVariant *parameters)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int allow_duplicate;
	int sender_pid;

	g_variant_get(parameters, "(ississi)", &sender_pid, &appid, &name, &type, &content, &icon, &allow_duplicate);
	_callback_info.request_cb(appid, name, type, content, icon, sender_pid, -1.0f, allow_duplicate, _callback_info.data);
}

static void _add_shortcut_widget_notify(GVariant *parameters)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int allow_duplicate;
	int sender_pid;
	double period;

	g_variant_get(parameters, "(ississdi)", &sender_pid, &appid, &name, &type, &content, &icon, &period, &allow_duplicate);
	_callback_info.request_cb(appid, name, type, content, icon, sender_pid, period, allow_duplicate, _callback_info.data);
}

static void _handle_shortcut_notify(GDBusConnection *connection,
		const gchar     *sender_name,
		const gchar     *object_path,
		const gchar     *interface_name,
		const gchar     *signal_name,
		GVariant        *parameters,
		gpointer         user_data)
{
	DbgPrint("signal_name: %s", signal_name);
	if (g_strcmp0(signal_name, "add_shortcut_notify") == 0)
		_add_shortcut_notify(parameters);
	else if (g_strcmp0(signal_name, "add_shortcut_widget_notify") == 0)
		_add_shortcut_widget_notify(parameters);
}

static int _dbus_init(void) {
	int ret = SHORTCUT_ERROR_NONE;
	GError *error = NULL;

	_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (_gdbus_conn == NULL) {
		ret = SHORTCUT_ERROR_IO_ERROR;
		if (error != NULL) {
			ErrPrint("Failed to get dbus [%s]", error->message);
			g_error_free(error);
		}
		goto out;
	}
out:
	if (!_gdbus_conn)
		g_object_unref(_gdbus_conn);

	return ret;
}

static int shortcut_dbus_init() {
	int ret = SHORTCUT_ERROR_NONE;
	int id = 0;

	if (_gdbus_conn == NULL) {
		ret = _dbus_init();
		if (ret == SHORTCUT_ERROR_NONE) {
			DbgPrint("get dbus connection success");
			id = g_dbus_connection_signal_subscribe(
					_gdbus_conn,
					PROVIDER_BUS_NAME,
					PROVIDER_SHORTCUT_INTERFACE_NAME,	/*    interface */
					NULL,					/*    member */
					PROVIDER_OBJECT_PATH,			/*    path */
					NULL,					/*    arg0 */
					G_DBUS_SIGNAL_FLAGS_NONE,
					_handle_shortcut_notify,
					NULL,
					NULL);

			DbgPrint("subscribe id : %d", id);
			if (id == 0) {
				ret = SHORTCUT_ERROR_IO_ERROR;
				ErrPrint("Failed to _register_noti_dbus_interface");
			} else {
				monitor_id = id;
			}
		}
	}
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
		if (fd < 0)
			return NULL;

		ret = read(fd, pkgname, sizeof(pkgname) - 1);
		close(fd);

		if (ret <= 0)
			return NULL;

		pkgname[ret] = '\0';
		/*!
		 * \NOTE
		 * "ret" is not able to be larger than "sizeof(pkgname) - 1",
		 * if the system is not going wrong.
		 */
	} else {
		if (strlen(pkgname) <= 0)
			return NULL;
	}

	dup_pkgname = strdup(pkgname);
	if (!dup_pkgname)
		ErrPrint("Heap: %d\n", errno);

	return dup_pkgname;
}

/*
 * implement user request
 */
int _send_sync_noti(GVariant *body, GDBusMessage **reply, char *cmd) {
	GError *err = NULL;
	GDBusMessage *msg = NULL;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_SHORTCUT_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ErrPrint("Can't allocate new method call");
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	if (body != NULL)
		g_dbus_message_set_body(msg, body);
	*reply = g_dbus_connection_send_message_with_reply_sync(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			&err);

	if (!*reply) {
		if (err != NULL) {
			ErrPrint("No reply. error = %s", err->message);
			g_error_free(err);
		}
		//		if (notification_ipc_is_master_ready() == 1)
		//			return NOTIFICATION_ERROR_PERMISSION_DENIED;
		//		else
		return SHORTCUT_ERROR_COMM;
	}
	ErrPrint("_send_sync_noti done !!");
	return SHORTCUT_ERROR_NONE;

}

int _send_service_register()
{
	GVariant *body = NULL;
	GDBusMessage *reply = NULL;
	int result = SHORTCUT_ERROR_NONE;

	result = _send_sync_noti(NULL, &reply, "shortcut_service_register");
	if (result == SHORTCUT_ERROR_NONE) {
		body = g_dbus_message_get_body(reply);
		g_variant_get(body, "(i)", &result);
	}

	ErrPrint("_send_service_register done = %s", _bus_name);
	return result;
}

static void _send_message_with_reply_sync_cb(GDBusConnection *connection,
		GAsyncResult *res,
		gpointer user_data)
{
	GVariant *body = NULL;
	int result = 0;
	GError *err = NULL;
	struct result_cb_item *cb_item = (struct result_cb_item *)user_data;
	GDBusMessage *reply = g_dbus_connection_send_message_with_reply_finish(
			connection,
			res,
			&err);
	body = g_dbus_message_get_body(reply);
	g_variant_get(body, "(i)", &result);

	if (cb_item->result_internal_cb)
		result = cb_item->result_internal_cb(result, getpid(), cb_item->data);
	else if (cb_item->result_cb)
		result = cb_item->result_cb(result, cb_item->data);
}

int _send_async_noti(GVariant *body, struct result_cb_item *cb_item, char *cmd)
{
	GDBusMessage *msg = NULL;
	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_SHORTCUT_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ErrPrint("Can't allocate new method call");
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	if (body != NULL)
		g_dbus_message_set_body(msg, body);

	g_dbus_connection_send_message_with_reply(
			_gdbus_conn,
			msg,
			G_DBUS_SEND_MESSAGE_FLAGS_NONE,
			-1,
			NULL,
			NULL,
			(GAsyncReadyCallback)_send_message_with_reply_sync_cb,
			cb_item);

	DbgPrint("_send_async_noti done !!");
	return SHORTCUT_ERROR_NONE;
}

static void _on_name_appeared(GDBusConnection *connection,
		const gchar     *name,
		const gchar     *name_owner,
		gpointer         user_data)
{
	DbgPrint("name appeared : %s", name);
	_send_service_register();
}

static void _on_name_vanished(GDBusConnection *connection,
		const gchar     *name,
		gpointer         user_data)
{
	DbgPrint("name vanished : %s", name);
}

EAPI int shortcut_set_request_cb(shortcut_request_cb request_cb, void *data)
{
	int ret = shortcut_dbus_init();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", ret);
		return ret;
	}

	if (request_cb == NULL)
		return SHORTCUT_ERROR_INVALID_PARAMETER;

	if (provider_monitor_id == 0) {
		provider_monitor_id = g_bus_watch_name_on_connection(
				_gdbus_conn,
				PROVIDER_BUS_NAME,
				G_BUS_NAME_WATCHER_FLAGS_NONE,
				_on_name_appeared,
				_on_name_vanished,
				NULL,
				NULL);

		if (provider_monitor_id == 0) {
			ErrPrint("watch on name fail");
			return SHORTCUT_ERROR_IO_ERROR;
		}
	}

	_callback_info.request_cb = request_cb;
	_callback_info.data = data;

	return SHORTCUT_ERROR_NONE;
}

EAPI int shortcut_add_to_home(const char *name, shortcut_type type, const char *uri,
		const char *icon, int allow_duplicate, result_cb_t result_cb, void *data)
{
	struct result_cb_item *item;
	char *appid = NULL;
	int ret = SHORTCUT_ERROR_NONE;
	GVariant *body = NULL;

	if (ADD_TO_HOME_IS_DYNAMICBOX(type)) {
		ErrPrint("Invalid type used for adding a shortcut\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	ret = shortcut_dbus_init();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", ret);
		return ret;
	}

	appid = _shortcut_get_pkgname_by_pid();
	item = malloc(sizeof(*item));
	if (!item) {
		if (appid)
			free(appid);

		ErrPrint("Heap: %d\n", errno);
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_cb = result_cb;
	item->result_internal_cb = NULL;
	item->data = data;

	if (!name)
		name = "";

	if (!uri)
		uri = "";

	if (!icon)
		icon = "";

	body = g_variant_new("(ississi)", getpid(), appid, name, type, uri, icon, allow_duplicate);
	ret = _send_async_noti(body, item, "add_shortcut");

	if (appid)
		free(appid);

	return ret;
}

EAPI int shortcut_add_to_home_widget(const char *name, shortcut_widget_size_e size, const char *widget_id,
		const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data)
{
	struct result_cb_item *item;
	char *appid = NULL;
	int ret = SHORTCUT_ERROR_NONE;
	GVariant *body = NULL;

	if (name == NULL) {
		ErrPrint("AppID is null\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!SHORTCUT_IS_WIDGET_SIZE(size)) {
		ErrPrint("Invalid type used for adding a widget\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	ret = shortcut_dbus_init();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", ret);
		return ret;
	}

	appid = _shortcut_get_pkgname_by_pid();
	item = malloc(sizeof(*item));
	if (!item) {
		if (appid)
			free(appid);

		ErrPrint("Heap: %d\n", errno);
		return SHORTCUT_ERROR_OUT_OF_MEMORY;
	}

	item->result_cb = result_cb;
	item->result_internal_cb = NULL;
	item->data = data;

	body = g_variant_new("(ississdi)", getpid(), widget_id, name, size, NULL, icon, period, allow_duplicate);
	ret = _send_async_noti(body, item, "add_shortcut_widget");

	if (appid)
		free(appid);

	return ret;
}

EAPI int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content,
		const char *icon, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	/*Deprecated API */
	return SHORTCUT_ERROR_NONE;
}

EAPI int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data)
{
	/*Deprecated API */
	return SHORTCUT_ERROR_NONE;
}

static inline int open_db(void)
{
	int ret;

	ret = db_util_open(dbfile, &handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		DbgPrint("Failed to open a %s\n", dbfile);
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

	status = sqlite3_prepare_v2(handle, query, -1, &stmt, NULL);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to prepare stmt: %s\n", sqlite3_errmsg(handle));
		return -EFAULT;
	}

	status = sqlite3_bind_int(stmt, 1, id);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind id: %s\n", sqlite3_errmsg(handle));
		ret = -EFAULT;
		goto out;
	}

	status = sqlite3_bind_text(stmt, 2, lang, -1, SQLITE_TRANSIENT);
	if (status != SQLITE_OK) {
		ErrPrint("Failed to bind lang: %s\n", sqlite3_errmsg(handle));
		ret = -EFAULT;
		goto out;
	}

	DbgPrint("id: %d, lang: %s\n", id, lang);
	if (SQLITE_ROW != sqlite3_step(stmt)) {
		ErrPrint("Failed to do step: %s\n", sqlite3_errmsg(handle));
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
				if (name && *name)
					free(*name);

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
	char *ptr;

	language = vconf_get_str(VCONFKEY_LANGSET);
	if (language) {
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
			ErrPrint("Heap: %d\n", errno);
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

	if (list_cb == NULL)
		return SHORTCUT_ERROR_INVALID_PARAMETER;

	if (!db_opened)
		db_opened = (open_db() == 0);

	if (!db_opened) {
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
		ret = sqlite3_prepare_v2(handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(handle));
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}

		ret = sqlite3_bind_text(stmt, 1, package_name, -1, SQLITE_TRANSIENT);
		if (ret != SQLITE_OK) {
			ErrPrint("bind text: %s\n", sqlite3_errmsg(handle));
			sqlite3_finalize(stmt);
			free(language);
			return SHORTCUT_ERROR_IO_ERROR;
		}
	} else {
		query = "SELECT id, appid, name, extra_key, extra_data, icon FROM shortcut_service";
		ret = sqlite3_prepare_v2(handle, query, -1, &stmt, NULL);
		if (ret != SQLITE_OK) {
			ErrPrint("prepare: %s\n", sqlite3_errmsg(handle));
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
		/* if (get_i18n_name(language, id, &i18n_name, &i18n_icon) < 0) { */
			/* Okay, we can't manage this. just use the fallback string */
		/* } */
		get_i18n_name(language, id, &i18n_name, &i18n_icon);

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
