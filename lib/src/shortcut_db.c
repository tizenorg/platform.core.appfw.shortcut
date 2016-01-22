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
#include <shortcut_private.h>
#include <shortcut_db.h>
#include <vconf.h>
#include <vconf-keys.h>

static const char *dbfile = DB_PATH;
static sqlite3 *handle = NULL;
static int db_opened = 0;

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

EAPI int shortcut_db_get_list(const char *package_name, GList **shortcut_list)
{
	sqlite3_stmt *stmt;
	const char *query;
	const unsigned char *name;
	char *i18n_name = NULL;
	char *i18n_icon = NULL;
	const unsigned char *extra_data;
	const unsigned char *extra_key;
	const unsigned char *icon;
	shortcut_info_s *shortcut;
	int id;
	int ret;
	int cnt;
	char *language;

	*shortcut_list = NULL;

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
		shortcut = (shortcut_info_s *)calloc(sizeof(shortcut_info_s), 1);
		if (shortcut == NULL) {
			free(i18n_name);
			break;
		}
		shortcut->package_name = strdup(package_name);
		shortcut->icon = strdup((i18n_icon != NULL ? i18n_icon : (char *)icon));
		shortcut->name = strdup((i18n_name != NULL ? i18n_name : (char *)name));
		shortcut->extra_key = strdup((char *)extra_key);
		shortcut->extra_data = strdup((char *)extra_key);
		*shortcut_list = g_list_append(*shortcut_list, shortcut);

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
