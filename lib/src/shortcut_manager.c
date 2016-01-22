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

#define SHORTCUT_SERVICE_BUS_NAME "org.tizen.shortcut_service"
#define SHORTCUT_SERVICE_INTERFACE_NAME "org.tizen.shortcut_service"
#define SHORTCUT_SERVICE_OBJECT_PATH "/org/tizen/shortcut_service"

#define PROVIDER_BUS_NAME "org.tizen.data_provider_service"
#define PROVIDER_OBJECT_PATH "/org/tizen/data_provider_service"
#define PROVIDER_SHORTCUT_INTERFACE_NAME "org.tizen.data_provider_shortcut_service"

#define SHORTCUT_IPC_DBUS_PREFIX "org.tizen.shortcut_ipc_"
#define SHORTCUT_IPC_OBJECT_PATH "/org/tizen/shortcut_service"

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"

static char *_bus_name = NULL;
static GDBusConnection *_gdbus_conn = NULL;
static int monitor_id = 0;

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

static void _add_shortcut_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
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

static void _add_shortcut_widget_notify(GVariant *parameters, GDBusMethodInvocation *invocation)
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

static void _dbus_method_call_handler(GDBusConnection *conn,
		const gchar *sender, const gchar *object_path, const gchar *iface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation, gpointer user_data)
{
	DbgPrint("method_name: %s", method_name);
	if (g_strcmp0(method_name, "add_shortcut_notify") == 0)
		_add_shortcut_notify(parameters, invocation);
	else if (g_strcmp0(method_name, "add_shortcut_widget_notify") == 0)
		_add_shortcut_widget_notify(parameters, invocation);
}

static char *_get_encoded_name(const char *appid) {
	int prefix_len = strlen(SHORTCUT_IPC_DBUS_PREFIX);

	unsigned char c[MD5_DIGEST_LENGTH] = { 0 };
	char *md5_interface = NULL;
	char *temp;
	int index = 0;
	MD5_CTX mdContext;
	int encoded_name_len = prefix_len + (MD5_DIGEST_LENGTH * 2) + 2;
	int appid_len = strlen(appid) + 1;

	MD5_Init(&mdContext);
	MD5_Update(&mdContext, appid, appid_len);
	MD5_Final(c, &mdContext);

	md5_interface = (char *) calloc(encoded_name_len, sizeof(char));
	if (md5_interface == NULL) {
		ErrPrint("md5_interface calloc failed!!");
		return 0;
	}

	snprintf(md5_interface, encoded_name_len, "%s", SHORTCUT_IPC_DBUS_PREFIX);
	temp = md5_interface;
	temp += prefix_len;

	for (index = 0; index < MD5_DIGEST_LENGTH; index++) {
		snprintf(temp, 3, "%02x", c[index]);
		temp += 2;
	}

	DbgPrint("encoded_name : %s ", md5_interface);
	return md5_interface;
}

static const GDBusInterfaceVTable interface_vtable = {
		_dbus_method_call_handler,
		NULL,
		NULL };

static void _shortcut_on_bus_acquired(GDBusConnection *connection,
		const gchar *name, gpointer user_data) {
	ErrPrint("_shortcut_on_bus_acquired : %s", name);
}

static void _shortcut_on_name_acquired(GDBusConnection *connection,
		const gchar *name, gpointer user_data) {
	ErrPrint("_shortcut_on_name_acquired : %s", name);
}

static void _shortcut_on_name_lost(GDBusConnection *connection, const gchar *name,
		gpointer user_data) {
	ErrPrint("_shortcut_on_name_lost : %s", name);
}

static int _register_shortcut_dbus_interface(const char *appid) {
	GDBusNodeInfo *introspection_data = NULL;
	int registration_id = 0;
	static gchar introspection_prefix[] = "<node>"
			"  <interface name='";
	static gchar introspection_postfix[] =
			"'>"
			"        <method name='add_shortcut_notify'>"
			"          <arg type='i' name='sender_pid' direction='in'/>"
			"          <arg type='s' name='appid' direction='in'/>"
			"          <arg type='s' name='name' direction='in'/>"
			"          <arg type='i' name='type' direction='in'/>"
			"          <arg type='s' name='content' direction='in'/>"
			"          <arg type='s' name='icon' direction='in'/>"
			"          <arg type='i' name='allow_duplicate' direction='in'/>"
			"        </method>"
			"        <method name='add_shortcut_widget_notify'>"
			"          <arg type='i' name='sender_pid' direction='in'/>"
			"          <arg type='s' name='appid' direction='in'/>"
			"          <arg type='s' name='name' direction='in'/>"
			"          <arg type='i' name='type' direction='in'/>"
			"          <arg type='s' name='content' direction='in'/>"
			"          <arg type='s' name='icon' direction='in'/>"
			"          <arg type='d' name='period' direction='in'/>"
			"          <arg type='i' name='allow_duplicate' direction='in'/>"
			"        </method>"
			"  </interface>"
			"</node>";
	char *introspection_xml = NULL;
	int introspection_xml_len = 0;
	int owner_id = 0;
	GVariant *result = NULL;
	char *interface_name = NULL;

	_bus_name = _get_encoded_name(appid);
	interface_name = _bus_name;

	introspection_xml_len = strlen(introspection_prefix)
			+ strlen(interface_name) + strlen(introspection_postfix) + 1;

	introspection_xml = (char *) calloc(introspection_xml_len, sizeof(char));
	if (!introspection_xml) {
		ErrPrint("out of memory");
		goto out;
	}

	owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, _bus_name,
			G_BUS_NAME_OWNER_FLAGS_NONE, _shortcut_on_bus_acquired,
			_shortcut_on_name_acquired, _shortcut_on_name_lost,
			NULL, NULL);
	if (!owner_id) {
		ErrPrint("g_bus_own_name error");
		g_dbus_node_info_unref(introspection_data);
		goto out;
	}

	DbgPrint("Acquiring the own name : %d", owner_id);

	snprintf(introspection_xml, introspection_xml_len, "%s%s%s",
			introspection_prefix, interface_name, introspection_postfix);

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	if (!introspection_data) {
		ErrPrint("g_dbus_node_info_new_for_xml() is failed.");
		goto out;
	}

	registration_id = g_dbus_connection_register_object(
			_gdbus_conn,
			SHORTCUT_SERVICE_OBJECT_PATH,
			introspection_data->interfaces[0],
			&interface_vtable, NULL, NULL, NULL);

	DbgPrint("registration_id %d", registration_id);
	if (registration_id == 0) {
		ErrPrint("Failed to g_dbus_connection_register_object");
		goto out;
	}
	out: if (introspection_data)
		g_dbus_node_info_unref(introspection_data);
	if (introspection_xml)
		free(introspection_xml);
	if (result)
		g_variant_unref(result);

	return registration_id;
}

static int _dbus_init(void) {
	bool ret = false;
	GError *error = NULL;

	_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (_gdbus_conn == NULL) {
		if (error != NULL) {
			ErrPrint("Failed to get dbus [%s]", error->message);
			g_error_free(error);
		}
		goto out;
	}
	ret = true;
	out: if (!_gdbus_conn)
		g_object_unref(_gdbus_conn);

	return ret;

}

static int shortcut_dbus_init() {
	int ret = SHORTCUT_ERROR_NONE;
	int id = 0;
	if (_gdbus_conn == NULL) {
		_dbus_init();
		id = _register_shortcut_dbus_interface(_shortcut_get_pkgname_by_pid());
		if (id < 1) {
			ret = SHORTCUT_ERROR_IO_ERROR;
			ErrPrint("Failed to _register_shortcut_dbus_interface");
		} else {
			monitor_id = id;
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

	body = g_variant_new("(s)", _bus_name);
	result = _send_sync_noti(body, &reply, "service_register");
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

EAPI int shortcut_set_request_cb(shortcut_request_cb request_cb, void *data)
{
	int ret = shortcut_dbus_init();
	if (ret != SHORTCUT_ERROR_NONE) {
		ErrPrint("Can't init dbus %d", ret);
		return ret;
	}

	if (request_cb == NULL)
		return SHORTCUT_ERROR_INVALID_PARAMETER;

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
