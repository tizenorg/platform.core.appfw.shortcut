
#ifndef __SHORTCUT_DB_H__
#define __SHORTCUT_DB_H__


#include <sqlite3.h>
#include <shortcut.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct shortcut_info {
	char *package_name;
	char *icon;
	char *name;
	char *extra_key;
	char *extra_data;
} shortcut_info_s;

extern int shortcut_db_get_list(const char *package_name, GList **shortcut_list);


#ifdef __cplusplus
}
#endif

#endif				/* __SHORTCUT_DB_H__ */