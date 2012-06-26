#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sqlite3.h>
#include <db-util.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "dlist.h"

/*!
 * DB Table schema
 *
 * +-------------+--------+---------+
 * | PackageName | NameID | Service |
 * +-------------+--------+---------+
 * |      -      |    -   |    -    |
 * +-------------+--------+---------+
 */

#if !defined(LIBXML_TREE_ENABLED)
	#error "LIBXML is not supporting the tree"
#endif

int errno;

static struct {
	struct dlist *node_list;
	const char *dbfile;
	sqlite3 *handle;
} s_info = {
	.node_list = NULL,
	.dbfile = "/opt/dbspace/.shortcut_service.db",
	.handle = NULL,
};

static inline void db_create_table(void)
{
	char *err;
	static const char *ddl =
		"CREATE TABLE shortcut_service ("
		"pkgname TEXT,"
		"name TEXT,"
		"service TEXT)";

	if (sqlite3_exec(s_info.handle, ddl, NULL, NULL, &err) != SQLITE_OK) {
		fprintf(stderr, "Failed to execute the DDL (%s)\n", err);
		return;
	}

	if (sqlite3_changes(s_info.handle) == 0)
		fprintf(stderr, "No changes to DB\n");
}

static inline int db_remove_record(const char *pkgname, const char *name, const char *service)
{
	static const char *dml = "DELETE FROM shortcut_service WHERE pkgname = ? AND name = ? AND service = ?";
	sqlite3_stmt *stmt;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare the initial DML\n");
		return -EIO;
	}

	ret = -EIO;
	if (sqlite3_bind_text(stmt, 1, pkgname, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a pkgname(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	if (sqlite3_bind_text(stmt, 2, name, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a name(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	if (sqlite3_bind_text(stmt, 3, service, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a service(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = 0;
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		ret = -EIO;
		fprintf(stderr, "Failed to execute the DML for %s - %s\n", pkgname, name);
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_insert_record(const char *pkgname, const char *name, const char *service)
{
	static const char *dml = "INSERT INTO shortcut_service (pkgname, name, service) VALUES (?, ?, ?)";
	sqlite3_stmt *stmt;
	int ret;

	ret = sqlite3_prepare_v2(s_info.handle, dml, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare the initial DML\n");
		return -EIO;
	}

	ret = -EIO;
	if (sqlite3_bind_text(stmt, 1, pkgname, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a pkgname(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	if (sqlite3_bind_text(stmt, 2, name, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a name(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	if (sqlite3_bind_text(stmt, 3, service, -1, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind a service(%s)\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = 0;
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		fprintf(stderr, "Failed to execute the DML for %s - %s\n", pkgname, name);
		ret = -EIO;
	}

out:
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

static inline int db_init(void)
{
	int ret;
	struct stat stat;

	ret = db_util_open(s_info.dbfile, &s_info.handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		fprintf(stderr, "Failed to open a DB\n");
		return -EIO;
	}

	if (lstat(s_info.dbfile, &stat) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		return -EIO;
	}

	if (!S_ISREG(stat.st_mode)) {
		fprintf(stderr, "Invalid file\n");
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

int uninstall_shortcut(const char *pkgname, xmlNodePtr root)
{
	int tap = 0;
	xmlNodePtr node = NULL;
	struct dlist *l;
	const char *name;
	const char *service;

	if (strcmp(root->name, "shortcut")) {
		fprintf(stderr, "Invalid XML root\n");
		return -EINVAL;
	}

	printf("Package: %s\n", pkgname);
	s_info.node_list = dlist_append(s_info.node_list, root->children);

	while (s_info.node_list) {
		l = dlist_nth(s_info.node_list, 0);
		root = dlist_data(l);
		s_info.node_list = dlist_remove(s_info.node_list, l);

		for (node = root; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE)
				printf("%*cElement %s\n", tap, ' ', node->name);

			if (!strcmp(node->name, "text"))
				continue;

			if (!xmlHasProp(node, "name") || !xmlHasProp(node, "service") || !xmlHasProp(node, "pkgname")) {
				printf("Invalid element %s\n", node->name);
				continue;
			}

			pkgname = xmlGetProp(node, "pkgname");
			name = xmlGetProp(node, "name");
			service = xmlGetProp(node, "service");

			printf("%*cpkgname: %s\n", tap + 3, ' ', pkgname);
			printf("%*cname_id: %s\n", tap + 3, ' ', name);
			printf("%*cservice: %s\n", tap + 3, ' ', service);

			if (db_remove_record(pkgname, name, service) < 0)
				fprintf(stderr, "Failed to insert a new record\n");

			if (node->children)
				printf("Skip this node's children\n");
		}

		tap += 3;
	}

	return 0;
}

int install_shortcut(const char *pkgname, xmlNodePtr root)
{
	int tap = 0;
	xmlNodePtr node = NULL;
	struct dlist *l;
	const char *name;
	const char *service;

	if (strcmp(root->name, "shortcut")) {
		fprintf(stderr, "Invalid XML root\n");
		return -EINVAL;
	}

	printf("Package: %s\n", pkgname);
	s_info.node_list = dlist_append(s_info.node_list, root->children);

	while (s_info.node_list) {
		l = dlist_nth(s_info.node_list, 0);
		root = dlist_data(l);
		s_info.node_list = dlist_remove(s_info.node_list, l);

		for (node = root; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE)
				printf("%*cElement %s\n", tap, ' ', node->name);

			if (!strcmp(node->name, "text"))
				continue;

			if (!xmlHasProp(node, "name") || !xmlHasProp(node, "service") || !xmlHasProp(node, "pkgname")) {
				printf("Invalid element %s\n", node->name);
				continue;
			}

			pkgname = xmlGetProp(node, "pkgname");
			name = xmlGetProp(node, "name");
			service = xmlGetProp(node, "service");

			printf("%*cpkgname: %s\n", tap + 3, ' ', pkgname);
			printf("%*cname_id: %s\n", tap + 3, ' ', name);
			printf("%*cservice: %s\n", tap + 3, ' ', service);

			if (db_insert_record(pkgname, name, service) < 0)
				fprintf(stderr, "Failed to insert a new record\n");

			if (node->children)
				printf("Skip this node's children\n");
		}

		tap += 3;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	xmlDoc *doc;
	xmlNode *root;

	if (argc != 2) {
		fprintf(stderr, "Invalid argument: %s XML_FILENAME\n", argv[0]);
		return -EINVAL;
	}

	doc = xmlReadFile(argv[1], NULL, 0);
	if (!doc) {
		fprintf(stderr, "Failed to parse %s\n", argv[1]);
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

/* End of a file */
