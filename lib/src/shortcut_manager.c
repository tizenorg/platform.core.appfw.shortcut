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

#include "shortcut.h"
#include "shortcut_private.h"
#include "shortcut_db.h"
#include "shortcut_manager.h"

#define SHORTCUT_PKGNAME_LEN 512
#define SHORTCUT_IS_WIDGET_SIZE(size)           (!!((size) & WIDGET_SIZE_DEFAULT))

#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_SHORTCUT_INTERFACE_NAME "org.tizen.data_provider_shortcut_service"

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"

static GDBusConnection *_gdbus_conn = NULL;
static int monitor_id = 0;
static int provider_monitor_id = 0;

static const GDBusErrorEntry dbus_error_entries[] =
{
	{SHORTCUT_ERROR_INVALID_PARAMETER, "org.freedesktop.Shortcut.Error.INVALID_PARAMETER"},
	{SHORTCUT_ERROR_OUT_OF_MEMORY,     "org.freedesktop.Shortcut.Error.OUT_OF_MEMORY"},
	{SHORTCUT_ERROR_IO_ERROR,          "org.freedesktop.Shortcut.Error.IO_ERROR"},
	{SHORTCUT_ERROR_PERMISSION_DENIED, "org.freedesktop.Shortcut.Error.PERMISSION_DENIED"},
	{SHORTCUT_ERROR_NOT_SUPPORTED,           "org.freedesktop.Shortcut.Error.NOT_SUPPORTED"},
	{SHORTCUT_ERROR_RESOURCE_BUSY,  "org.freedesktop.Shortcut.Error.RESOURCE_BUSY"},
	{SHORTCUT_ERROR_NO_SPACE,         "org.freedesktop.Shortcut.Error.NO_SPACE"},
	{SHORTCUT_ERROR_EXIST,      "org.freedesktop.Shortcut.Error.EXIST"},
	{SHORTCUT_ERROR_FAULT, "org.freedesktop.Shortcut.Error.FAULT"},
	{SHORTCUT_ERROR_COMM, "org.freedesktop.Shortcut.Error.COMM"},
};

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
static int _dbus_init();
static char *_shortcut_get_pkgname_by_pid(void);

EXPORT_API GQuark shortcut_error_quark (void)
{
	static volatile gsize quark_volatile = 0;
	g_dbus_error_register_error_domain ("shortcut-error-quark",
			&quark_volatile,
			dbus_error_entries,
			G_N_ELEMENTS(dbus_error_entries));
	return (GQuark) quark_volatile;
}

static void _add_shortcut_notify(GVariant *parameters)
{
	const char *appid;
	const char *name;
	int type;
	const char *content;
	const char *icon;
	int allow_duplicate;
	int sender_pid;

	g_variant_get(parameters, "(i&s&si&s&si)", &sender_pid, &appid, &name, &type, &content, &icon, &allow_duplicate);
	DbgPrint("_add_shortcut_notify sender_pid: %d ", sender_pid);
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

	g_variant_get(parameters, "(i&s&si&s&sdi)", &sender_pid, &appid, &name, &type, &content, &icon, &period, &allow_duplicate);
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

static int _dbus_init(void)
{
	GError *error = NULL;

	if (_gdbus_conn == NULL) {
		_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

		if (_gdbus_conn == NULL) {
			if (error != NULL) {
				ErrPrint("Failed to get dbus [%s]", error->message);
				g_error_free(error);
			}
			return SHORTCUT_ERROR_IO_ERROR;
		}
		shortcut_error_quark();
	}

	return SHORTCUT_ERROR_NONE;
}

static int _dbus_signal_init()
{
	int ret = SHORTCUT_ERROR_NONE;
	int id;

	if (monitor_id == 0) {
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

	return ret;
}

static char *_shortcut_get_pkgname_by_pid(void)
{
	char pkgname[SHORTCUT_PKGNAME_LEN + 1] = { 0, };
	char buf[SHORTCUT_PKGNAME_LEN + 1] = { 0, };
	int pid, ret;
	int fd;
	char *dup_pkgname;

	pid = getpid();

	ret = aul_app_get_pkgname_bypid(pid, pkgname, sizeof(pkgname));
	if (ret != 0) {
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
static int _send_sync_shortcut(GVariant *body, GDBusMessage **reply, char *cmd)
{
	GError *err = NULL;
	GDBusMessage *msg ;
	int ret = SHORTCUT_ERROR_NONE;

	msg = g_dbus_message_new_method_call(
			PROVIDER_BUS_NAME,
			PROVIDER_OBJECT_PATH,
			PROVIDER_SHORTCUT_INTERFACE_NAME,
			cmd);
	if (!msg) {
		ErrPrint("Can't allocate new method call");
		if (body)
			g_variant_unref(body);
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
		return SHORTCUT_ERROR_COMM;
	}

	g_object_unref(msg);

	if (g_dbus_message_to_gerror(*reply, &err)) {
		ret = err->code;
		ErrPrint("_send_sync_shortcut error %s", err->message);
		g_error_free(err);
		return ret;
	}
	DbgPrint("_send_sync_shortcut done !!");
	return SHORTCUT_ERROR_NONE;
}

static int _send_service_register()
{
	GDBusMessage *reply = NULL;
	int result;

	result = _send_sync_shortcut(NULL, &reply, "shortcut_service_register");
	ErrPrint("_send_service_register done");
	return result;
}

static void _send_message_with_reply_sync_cb(GDBusConnection *connection,
		GAsyncResult *res,
		gpointer user_data)
{
	int result = SHORTCUT_ERROR_NONE;
	GError *err = NULL;
	GDBusMessage *reply = NULL;
	struct result_cb_item *cb_item = (struct result_cb_item *)user_data;

	if (cb_item == NULL) {
		ErrPrint("Failed to get a callback item");
		return;
	}

	reply = g_dbus_connection_send_message_with_reply_finish(
			connection,
			res,
			&err);

	if (!reply) {
		if (err != NULL) {
			ErrPrint("No reply. error = %s", err->message);
			g_error_free(err);
		}
		result = SHORTCUT_ERROR_COMM;

	} else if (g_dbus_message_to_gerror(reply, &err)) {
		result = err->code;
		g_error_free(err);
		ErrPrint("_send_async_noti error %s", err->message);
	}

	if (cb_item->result_internal_cb)
		result = cb_item->result_internal_cb(result, getpid(), cb_item->data);
	else if (cb_item->result_cb)
		result = cb_item->result_cb(result, cb_item->data);

	if (reply)
		g_object_unref(reply);

	free(cb_item);
}

static int _send_async_noti(GVariant *body, struct result_cb_item *cb_item, char *cmd)
{
	GDBusMessage *msg;
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
	int ret = _dbus_init();

	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", ret);
		return ret;
	}

	ret = _dbus_signal_init();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus_signal %d", ret);
		return ret;
	}

	ret = _send_service_register();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init ipc_monitor_register %d", ret);
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
	char *appid;
	int ret;
	GVariant *body;

	if (ADD_TO_HOME_IS_DYNAMICBOX(type)) {
		ErrPrint("Invalid type used for adding a shortcut\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_init();
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
	if (ret != SHORTCUT_ERROR_NONE) {
		free(item);
		item = NULL;
	}

	if (appid)
		free(appid);
	if (body)
		g_variant_unref(body);

	return ret;
}

EAPI int shortcut_add_to_home_widget(const char *name, shortcut_widget_size_e size, const char *widget_id,
		const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data)
{
	struct result_cb_item *item;
	char *appid;
	int ret;
	GVariant *body;

	if (name == NULL) {
		ErrPrint("AppID is null\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	if (!SHORTCUT_IS_WIDGET_SIZE(size)) {
		ErrPrint("Invalid type used for adding a widget\n");
		return SHORTCUT_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_init();
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

	if (ret != SHORTCUT_ERROR_NONE) {
		free(item);
		item = NULL;
	}

	if (appid)
		free(appid);
	if (body)
		g_variant_unref(body);

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


EAPI int shortcut_get_list(const char *package_name, shortcut_list_cb list_cb, void *data)
{
	GDBusMessage *reply = NULL;
	int result;
	int count = 0;
	GVariant *build_body;
	GVariant *body;
	GVariant *reply_body;
	GVariant *iter_body;
	GVariantIter *iter;
	GVariantBuilder *b;
	shortcut_info_s shortcut;

	if (list_cb == NULL)
		return SHORTCUT_ERROR_INVALID_PARAMETER;

	result = _dbus_init();
	if (result != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", result);
		return result;
	}

	b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	if (package_name)
		g_variant_builder_add(b, "{sv}", "package_name", g_variant_new_string(package_name));
	build_body = g_variant_builder_end(b);
	body = g_variant_new("(v)", build_body);
	result = _send_sync_shortcut(body, &reply, "get_list");

	if (result == SHORTCUT_ERROR_NONE) {
		reply_body = g_dbus_message_get_body(reply);
		g_variant_get(reply_body, "(ia(v))", &count, &iter);

		while (g_variant_iter_loop(iter, "(v)", &iter_body)) {
			g_variant_get(reply_body, "(&s&s&s&s&s)",
				&shortcut.package_name, &shortcut.icon, &shortcut.name, &shortcut.extra_key, &shortcut.extra_data);
			list_cb(shortcut.package_name, shortcut.icon, shortcut.name, shortcut.extra_key, shortcut.extra_data, data);
		}
		g_variant_iter_free(iter);
	}

	if(reply)
		g_object_unref(reply);

	return count;
}

/* End of a file */
