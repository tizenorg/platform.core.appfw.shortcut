#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include <sqlite3.h>
#include <db-util.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <dlog.h>

#include "dlist.h"

#if !defined(FLOG)
#define DbgPrint(format, arg...)	LOGD("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#define ErrPrint(format, arg...)	LOGE("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#endif
/* End of a file */

/*!
 * \note
 * DB Table schema
 *
 * homescreen
 * +-------+------+------+------+
 * | appid | icon | name | desc |
 * +-------+------+------+------|
 * |   -   |      |      |      |
 * +-------+------+------+------+
 * CREATE TABLE homescreen ( appid TEXT PRIMARY KEY NOT NULL, icon TEXT, name TEXT, desc TEXT )
 *
 * image list
 * +-------+----+------------+
 * | appid | ID | image path |
 * +-------+----+------------+
 * |   -   |  - |      -     |
 * +-------+----+------------+
 * CREATE TABLE image ( appid TEXT NOT NULL, id INTEGER, path TEXT NOT NULL, FOREIGN KEY(appid) REFERENCES homescreen(appid) )
 *
 * description list
 * +-------+------+------+------+
 * | appid | lang | desc | name |
 * +-------+------+------+------+
 * |   -   |   -  |  -   |      |
 * +-------+------+------+------+
 * CREATE TABLE desc ( appid TEXT NOT NULL, lang TEXT NOT NULL, desc NOT NULL, name NOT NULL, FOREIGN KEY(appid) REFERENCES homescreen(appid) )
 *
 */

#if !defined(LIBXML_TREE_ENABLED)
	#error "LIBXML is not supporting the tree"
#endif

#if defined(LOG_TAG)
#undef LOG_TAG
#endif

#define LOG_TAG "pkgmgr_homescreen"

int errno;

static struct {
	const char *dbfile;
	sqlite3 *handle;
} s_info = {
	.dbfile = "/opt/dbspace/.shortcut_service.db",
	.handle = NULL,
};

static inline int begin_transaction(void)
{
	sqlite3_stmt *stmt;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, "BEGIN TRANSACTION", -1, &stmt, NULL);

	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return EXIT_FAILURE;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		ErrPrint("Failed to do update (%s)\n",
					sqlite3_errmsg(s_info.handle));
		sqlite3_finalize(stmt);
		return EXIT_FAILURE;
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

static inline int rollback_transaction(void)
{
	int ret;
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, "ROLLBACK TRANSACTION", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return EXIT_FAILURE;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		ErrPrint("Failed to do update (%s)\n",
				sqlite3_errmsg(s_info.handle));
		sqlite3_finalize(stmt);
		return EXIT_FAILURE;
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}

static inline int commit_transaction(void)
{
	sqlite3_stmt *stmt;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, "COMMIT TRANSACTION", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return EXIT_FAILURE;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		ErrPrint("Failed to do update (%s)\n",
					sqlite3_errmsg(s_info.handle));
		sqlite3_finalize(stmt);
		return EXIT_FAILURE;
	}

	sqlite3_finalize(stmt);
	return EXIT_SUCCESS;
}


static inline int db_create_homescreen(void)
{
	int ret;
	char *err;
	static const char *ddl = "CREATE TABLE homescreen ( appid TEXT PRIMARY KEY NOT NULL, icon TEXT, name TEXT, desc TEXT )";

	ret = sqlite3_exec(s_info.handle, ddl, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		ErrPrint("Failed to execute the DDL(%s)\n", err);
		return -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		ErrPrint("No changes to DB\n");

	return 0;
}

static inline int db_insert_homescreen(const char *appid, const char *icon, const char *name, const char *desc)
{
	int ret;
	static const char *dml = "INSERT INTO homescreen (appid, icon, name, desc) VALUES (?, ?, ?, ?)";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, icon, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 3, name, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 4, desc, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_remove_homescreen(const char *appid)
{
	int ret;
	static const char *dml = "DELETE FROM homescreen WHERE appid = ?";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		DbgPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_update_homescreen(const char *appid, const char *icon, const char *name, const char *desc)
{
	int ret;
	static const char *dml = "UPDATE homescreen SET icon = ?, name = ?, desc = ? WHERE appid = ?";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, icon, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, name, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 3, desc, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 4, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (sqlite3_changes(s_info.handle) == 0)
		DbgPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_create_image(void)
{
	int ret;
	char *err;
	static const char *ddl = "CREATE TABLE image ( appid TEXT NOT NULL, id INTEGER, path TEXT NOT NULL, FOREIGN KEY(appid) REFERENCES homescreen(appid) )";

	ret = sqlite3_exec(s_info.handle, ddl, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		ErrPrint("Failed to execute the DDL(%s)\n", err);
		return -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		ErrPrint("No changes to DB\n");

	return 0;
}

static inline int db_insert_image(const char *appid, int id, const char *path)
{
	int ret;
	static const char *dml = "INSERT INTO image (appid, id, path) VALUES (?, ?, ?)";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_int(stmt, 2, id);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 3, path, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_remove_image(const char *appid)
{
	int ret;
	static const char *dml = "DELETE FROM image WHERE appid = ?";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		DbgPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_update_image(const char *appid, int id, const char *path)
{
	int ret;
	static const char *dml = "UPDATE image SET path = ? WHERE appid = ? AND id = ?";
	sqlite3_stmt *stmt;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, path, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_int(stmt, 3, id);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		DbgPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_create_desc(void)
{
	int ret;
	char *err;
	static const char *ddl = "CREATE TABLE desc ( appid TEXT NOT NULL, lang TEXT NOT NULL, desc NOT NULL, name NOT NULL, FOREIGN KEY(appid) REFERENCES homescreen(appid) )";

	ret = sqlite3_exec(s_info.handle, ddl, NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		ErrPrint("Failed to execute the DDL(%s)\n", err);
		return -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		ErrPrint("No changes to DB\n");

	return 0;
}

static inline int db_insert_desc(const char *appid, const char *lang, const char *desc, const char *name)
{
	int ret;
	static const char *dml = "INSERT INTO desc ( appid, lang, desc, name ) VALUES ( ?, ?, ?, ? )";
	sqlite3_stmt *stmt;

	if (!appid)
		return -EINVAL;

	if (!lang)
		return -EINVAL;

	if (!desc)
		desc = "";

	if (!name)
		name = "";

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, lang, -1, NULL);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 3, desc, -1, NULL);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 4, name, -1, NULL);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_remove_desc(const char *appid)
{
	int ret;
	static const char *dml;
	sqlite3_stmt *stmt;

	if (!appid)
		return -EINVAL;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = 0;
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		ErrPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_update_desc(const char *appid, const char *lang, const char *desc, const char *name)
{
	int ret;
	static const char *dml = "UPDATE desc SET desc = ?, name = ? WHERE appid = ? AND lang = ?";
	sqlite3_stmt *stmt;

	if (!appid)
		return -EINVAL;

	if (!lang)
		return -EINVAL;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	ret = sqlite3_bind_text(stmt, 1, desc, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, name, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 3, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 4, lang, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		ErrPrint("DB has no changes\n");

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline void db_create_table(void)
{
	int ret;
	begin_transaction();

	ret = db_create_homescreen();
	if (ret < 0) {
		rollback_transaction();
		return;
	}
		
	ret = db_create_image();
	if (ret < 0) {
		rollback_transaction();
		return;
	}

	ret = db_create_desc();
	if (ret < 0) {
		rollback_transaction();
		return;
	}

	commit_transaction();
}

static inline int db_init(void)
{
	int ret;
	struct stat stat;

	ret = db_util_open(s_info.dbfile, &s_info.handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		ErrPrint("Failed to open a DB\n");
		return -EIO;
	}

	if (lstat(s_info.dbfile, &stat) < 0) {
		ErrPrint("%s\n", strerror(errno));
		return -EIO;
	}

	if (!S_ISREG(stat.st_mode)) {
		ErrPrint("Invalid file\n");
		return -EINVAL;
	}

	if (!stat.st_size)
		db_create_table();

	return 0;
}

static inline int db_fini(void)
{
	if (!s_info.handle)
		return 0;

	db_util_close(s_info.handle);
	s_info.handle = NULL;

	return 0;
}

struct desc {
	xmlChar *lang;
	xmlChar *desc;
	xmlChar *name;
};

struct image {
	int id;
	xmlChar *path;
};

struct homescreen {
	char *appid;
	xmlChar *icon;
	xmlChar *name;
	xmlChar *desc;
	struct dlist *image_list;
	struct dlist *desc_list;
};

static inline int homescreen_destroy(struct homescreen *homescreen)
{
	struct dlist *l;
	struct dlist *n;
	struct desc *desc;
	struct image *image;

	free(homescreen->appid);
	xmlFree(homescreen->icon);
	xmlFree(homescreen->name);
	xmlFree(homescreen->desc);

	dlist_foreach_safe(homescreen->image_list, l, n, image) {
		homescreen->image_list = dlist_remove(homescreen->image_list, l);
		xmlFree(image->path);
		free(image);
	}

	dlist_foreach_safe(homescreen->desc_list, l, n, desc) {
		homescreen->desc_list = dlist_remove(homescreen->desc_list, l);
		xmlFree(desc->lang);
		xmlFree(desc->desc);
		xmlFree(desc->name);
		free(desc);
	}

	return 0;
}

static inline int db_insert_item(struct homescreen *homescreen)
{
	return 0;
}

static inline int update_name(struct homescreen *homescreen, xmlNodePtr node)
{
	xmlChar *name;
	xmlChar *lang;
	struct dlist *l;
	struct desc *desc;

	name = xmlNodeGetContent(node);
	if (!name)
		return -EINVAL;

	lang = xmlNodeGetLang(node);
	if (!lang) {
		if (homescreen->name) {
			DbgPrint("Overwrite the name: %s\n", homescreen->name);
			xmlFree(homescreen->name);
		}

		homescreen->name = name;
		return 0;
	}

	dlist_foreach(homescreen->desc_list, l, desc) {
		if (!xmlStrcmp(desc->lang, lang)) {
			if (desc->name) {
				DbgPrint("Overwrite the name: %s\n", desc->name);
				xmlFree(desc->name);
			}

			desc->name = name;
			return 0;
		}
	}

	desc = calloc(1, sizeof(*desc));
	if (!desc) {
		ErrPrint("Heap: %s\n", strerror(errno));
		xmlFree(name);
		xmlFree(lang);
		return -ENOMEM;
	}

	desc->name = name;
	desc->lang = lang;
	desc->desc = NULL;

	homescreen->desc_list = dlist_append(homescreen->desc_list, desc);
	return 0;
}

static inline int update_icon(struct homescreen *homescreen, xmlNodePtr node)
{
	xmlChar *path;

	path = xmlNodeGetContent(node);
	if (!path)
		return -EINVAL;

	if (homescreen->icon) {
		DbgPrint("Overwrite icon: %s\n", homescreen->icon);
		xmlFree(homescreen->icon);
	}

	homescreen->icon = path;
	return 0;
}

static inline int update_image(struct homescreen *homescreen, xmlNodePtr node)
{
	xmlChar *path;
	xmlChar *id_str;
	int id;
	struct dlist *l;
	struct image *image;

	if (!xmlHasProp(node, (const xmlChar *)"src")) {
		DbgPrint("Has no source\n");
		return -EINVAL;
	}

	if (!xmlHasProp(node, (const xmlChar *)"id")) {
		DbgPrint("Has no id\n");
		return -EINVAL;
	}

	path = xmlGetProp(node, (const xmlChar *)"src");
	if (!path) {
		ErrPrint("Invalid path\n");
		return -EINVAL;
	}

	id_str = xmlGetProp(node, (const xmlChar *)"id");
	if (!id_str) {
		ErrPrint("Invalid id\n");
		xmlFree(path);
		return -EINVAL;
	}

	id = atoi((char *)id_str);
	xmlFree(id_str);

	dlist_foreach(homescreen->image_list, l, image) {
		if (image->id == id) {
			xmlFree(image->path);
			image->path = path;
			return 0;
		}
	}

	image = calloc(1, sizeof(*image));
	if (!image) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	image->path = path;
	image->id = id;

	homescreen->image_list = dlist_append(homescreen->image_list, image);
	return 0;
}

static inline int update_desc(struct homescreen *homescreen, xmlNodePtr node)
{
	xmlChar *desc_str;
	xmlChar *lang;
	struct desc *desc;
	struct dlist *l;

	desc_str = xmlNodeGetContent(node);
	if (!desc_str) {
		desc_str = xmlStrdup((const xmlChar *)"No description");
		if (!desc_str) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return -ENOMEM;
		}
	}

	lang = xmlNodeGetLang(node);
	if (!lang) {
		if (homescreen->desc) {
			DbgPrint("Overwrite desc: %s\n", homescreen->desc);
			xmlFree(homescreen->desc);
		}

		homescreen->desc = desc_str;
		return 0;
	}

	dlist_foreach(homescreen->desc_list, l, desc) {
		if (!xmlStrcmp(desc->lang, lang)) {
			if (desc->desc) {
				DbgPrint("Overwrite desc: %s\n", desc->desc);
				xmlFree(desc->desc);
			}

			desc->desc = desc_str;
			return 0;
		}
	}

	desc = calloc(1, sizeof(*desc));
	if (!desc) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	desc->lang = lang;
	desc->desc = desc_str;
	desc->name = NULL;

	homescreen->desc_list = dlist_append(homescreen->desc_list, desc);
	return 0;
}

int PKGMGR_PARSER_PLUGIN_UPGRADE(xmlDocPtr docPtr, const char *appid)
{
	xmlNodePtr node;

	if (!s_info.handle) {
		if (db_init() < 0) {
			ErrPrint("Failed to init DB\n");
			return -EIO;
		}
	}

	node = xmlDocGetRootElement(docPtr);
	if (!node) {
		ErrPrint("Invalid document\n");
		return -EINVAL;
	}

	for (node = node->children; node; node = node->next) {
		if (!xmlStrcasecmp(node->name, (const xmlChar *)"homescreen"))
			break;
	}

	if (!node) {
		ErrPrint("Root has no homescreen\n");
		return -EINVAL;
	}

	return 0;
}

int PKGMGR_PARSER_PLUGIN_INSTALL(xmlDocPtr docPtr, const char *pkgname)
{
	xmlNodePtr node;
	struct homescreen *homescreen;

	if (!s_info.handle) {
		if (db_init() < 0) {
			ErrPrint("Failed to init DB\n");
			return -EIO;
		}
	}

	node = xmlDocGetRootElement(docPtr);
	if (!node) {
		ErrPrint("Invalid document\n");
		return -EINVAL;
	}

	for (node = node->children; node; node = node->next) {
		if (!xmlStrcasecmp(node->name, (const xmlChar *)"homescreen"))
			break;
	}

	if (!node) {
		ErrPrint("Root has no children\n");
		return -EINVAL;
	}

	homescreen = calloc(1, sizeof(*homescreen));
	if (!homescreen) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	homescreen->appid = strdup(pkgname);
	if (!homescreen->appid) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return -ENOMEM;
	}

	for (node = node->children; node; node = node->next) {
		if (!xmlStrcmp(node->name, (const xmlChar *)"text"))
			continue;

		DbgPrint("Nodename: %s\n", node->name);
		if (!xmlStrcasecmp(node->name, (const xmlChar *)"label"))
			update_name(homescreen, node);
		else if (!xmlStrcasecmp(node->name, (const xmlChar *)"icon"))
			update_icon(homescreen, node);
		else if (!xmlStrcasecmp(node->name, (const xmlChar *)"img"))
			update_image(homescreen, node);
		else if (!xmlStrcasecmp(node->name, (const xmlChar *)"desc"))
			update_desc(homescreen, node);
		else
			ErrPrint("Unknown node: %s\n", node->name);
	}

	return db_insert_item(homescreen);
}

int PKGMGR_PARSER_PLUGIN_UNINSTALL(xmlDocPtr docPtr, const char *pkgname)
{
	xmlNodePtr node;
	int ret;

	if (!s_info.handle) {
		if (db_init() < 0) {
			ErrPrint("Failed to init DB\n");
			return -EIO;
		}
	}

	node = xmlDocGetRootElement(docPtr);
	if (!node) {
		ErrPrint("Invalid document\n");
		return -EINVAL;
	}

	for (node = node->children; node; node = node->next) {
		if (!xmlStrcasecmp(node->name, (const xmlChar *)"homescreen"))
			break;
	}

	if (!node) {
		ErrPrint("Root has no children\n");
		return -EINVAL;
	}

	begin_transaction();
	ret = db_remove_image(pkgname);
	if (ret < 0) {
		rollback_transaction();
		return ret;
	}

	ret = db_remove_desc(pkgname);
	if (ret < 0) {
		rollback_transaction();
		return ret;
	}

	ret = db_remove_homescreen(pkgname);
	if (ret < 0) {
		rollback_transaction();
		return ret;
	}

	commit_transaction();
	return 0;
}

/*
int main(int argc, char *argv[])
{
	xmlDoc *doc;
	xmlNode *root;

	if (argc != 2) {
		ErrPRint("Invalid argument: %s XML_FILENAME\n", argv[0]);
		return -EINVAL;
	}

	doc = xmlReadFile(argv[1], NULL, 0);
	if (!doc) {
		ErrPrint("Failed to parse %s\n", argv[1]);
		return -EIO;
	}

	root = xmlDocGetRootElement(doc);

	db_init();
	install_shortcut("", root);
	db_fini();

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return 0;
}
*/

/* End of a file */
