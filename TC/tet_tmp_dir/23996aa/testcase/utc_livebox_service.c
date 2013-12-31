/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <tet_api.h>
#include <stdlib.h>

#include <livebox-service.h>
#include <livebox-errno.h>

#define MUSIC_APP "com.samsung.music-player"
#define MUSIC_LIVEBOX MUSIC_APP ".livebox"
#define MUSIC_EASYBOX "com.samsung.music-player.easymode.livebox"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");

	// Create DB file for testing
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
	// Delete DB file for testing
}

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_livebox_service_get_size_n(void)
{
	int ret;
	int w;
	int h;

	ret = livebox_service_get_size(LB_SIZE_TYPE_UNKNOWN, &w, &h);
	dts_check_eq("livebox_service_get_size", ret, LB_STATUS_ERROR_INVALID, "invalid size should returns LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_service_get_size_p(void)
{
	int ret;
	int w;
	int h;
	ret = livebox_service_get_size(LB_SIZE_TYPE_1x1, &w, &h);
	dts_check_eq("livebox_service_get_size", ret, LB_STATUS_SUCCESS, "valid size should returns LB_STATUS_SUCCES\n");
}

static void utc_livebox_service_size_type_n(void)
{
	int ret;
	ret = livebox_service_size_type(1, 1);
	dts_check_eq("livebox_service_size_type", ret, LB_SIZE_TYPE_UNKNOWN, "invalid size should returns LB_SIZE_TYPE_UNKNOWN\n");
}

static void utc_livebox_service_size_type_p(void)
{
	int ret;
	int w;
	int h;
	int old;

	old = livebox_service_get_size(LB_SIZE_TYPE_1x1, &w, &h);

	ret = livebox_service_size_type(w, h);
	dts_check_eq("livebox_service_size_type", ret, LB_SIZE_TYPE_1x1, "LB_SIZE_TYPE_1x1 should returns\n");
}

static void utc_livebox_service_mouse_event_n(void)
{
	int ret;
	ret = livebox_service_mouse_event(NULL);
	dts_check_eq("livebox_service_mouse_event", ret, 0, "Invalid package name returns 0\n");
}

static void utc_livebox_service_mouse_event_p(void)
{
	int ret;
	ret = livebox_service_mouse_event(MUSIC_EASYBOX);
	dts_check_eq("livebox_service_mouse_event", ret, 1, "music player returns 1\n");
}

static void utc_livebox_service_touch_effect_n(void)
{
	int ret;
	ret = livebox_service_touch_effect(NULL, LB_SIZE_TYPE_1x1);
	dts_check_eq("livebox_service_touch_effect", ret, 1, "Default touch effect is 1\n");
}

static void utc_livebox_service_touch_effect_p(void)
{
	int ret;
	ret = livebox_service_touch_effect(MUSIC_LIVEBOX, LB_SIZE_TYPE_1x1);
	dts_check_eq("livebox_service_touch_effect", ret, 1, "Default touch effect is 1\n");
}

static void utc_livebox_service_need_frame_n(void)
{
	int ret;
	ret = livebox_service_need_frame(NULL, LB_SIZE_TYPE_1x1);
	dts_check_eq("livebox_service_need_frame", ret, 0, "Default need_frame is 0\n");
}

static void utc_livebox_service_need_frame_p(void)
{
	int ret;
	ret = livebox_service_need_frame(MUSIC_LIVEBOX, LB_SIZE_TYPE_1x1);
	dts_check_eq("livebox_service_need_frame", ret, 0, "Default need_frame is 0\n");
}

static void utc_livebox_service_trigger_update_n(void)
{
	int ret;
	ret = livebox_service_trigger_update(NULL, NULL, NULL, NULL, 0);
	dts_check_eq("livebox_service_trigger_update", ret, LB_STATUS_ERROR_INVALID, "Invalid livebox\n");
}

static void utc_livebox_service_trigger_update_p(void)
{
	int ret;
	ret = livebox_service_trigger_update(MUSIC_LIVEBOX, NULL, NULL, NULL, 0);
	dts_check_eq("livebox_service_trigger_update", ret, LB_STATUS_SUCCESS, "Invalid livebox\n");
}

static void utc_livebox_service_trigger_update_with_content_n(void)
{
	int ret;
	ret = livebox_service_trigger_update_with_content(NULL, NULL, NULL, NULL, NULL, 0);
	dts_check_eq("livebox_service_trigger_update_with_content", ret, LB_STATUS_ERROR_INVALID, "Invalid livebox\n");
}

static void utc_livebox_service_trigger_update_with_content_p(void)
{
	int ret;
	ret = livebox_service_trigger_update_with_content(MUSIC_LIVEBOX, NULL, NULL, NULL, NULL, 0);
	dts_check_eq("livebox_service_trigger_update_with_content", ret, LB_STATUS_ERROR_INVALID, "Invalid livebox\n");
}

static void utc_livebox_service_change_period_n(void)
{
	int ret;

	ret = livebox_service_change_period(NULL, NULL, 0.0f);
	dts_check_eq("livebox_service_change_period", ret, LB_STATUS_ERROR_INVALID, "Invalid livebox\n");
}

static void utc_livebox_service_change_period_p(void)
{
	int ret;

	ret = livebox_service_change_period(MUSIC_LIVEBOX, NULL, 1.0f);
	dts_check_eq("livebox_service_change_period", ret, LB_STATUS_SUCCESS, "Invalid livebox\n");
}

static int get_pkglist_cb(const char *pkgid, const char *lbid, int is_prime, void *data)
{
}

static void utc_livebox_service_get_pkglist_n(void)
{
	int ret;
	ret = livebox_service_get_pkglist(NULL, NULL);
	dts_check_eq("livebox_service_get_pkglist", ret, LB_STATUS_ERROR_INVALID, "Invalid callback\n");
}

static void utc_livebox_service_get_pkglist_p(void)
{
	int ret;
	ret = livebox_service_get_pkglist(get_pkglist_cb, NULL);
	dts_check_eq("livebox_service_get_pkglist", ret, LB_STATUS_SUCCESS, "Valid callback\n");
}

static void utc_livebox_service_get_applist_n(void)
{
	int ret;
	ret = livebox_service_get_applist(NULL, NULL, NULL);
	dts_check_eq("livebox_service_get_applist", ret, LB_STATUS_ERROR_INVALID, "LB_STATUS_ERROR_INVALID\n");
}

static void app_list_cb(const char *lbid, const char *appid, void *data)
{
}

static void utc_livebox_service_get_applist_p(void)
{
	int ret;

	// Getting the UI-App list
	ret = livebox_service_get_applist(MUSIC_LIVEBOX, app_list_cb, NULL);
	dts_check_eq("livebox_service_get_applist", ret, LB_STATUS_SUCCESS, "LB_STATUS_ERROR_INVALID\n");
}

static void utc_livebox_service_mainappid_n(void)
{
	const char *appid;
	appid = livebox_service_mainappid(NULL);
	dts_check_eq("livebox_service_mainappid", appid, NULL, "\"NULL\" should be returned\n");
}

static void utc_livebox_service_mainappid_p(void)
{
	const char *appid;
	appid = livebox_service_mainappid(MUSIC_LIVEBOX);
	dts_check_str_eq("livebox_service_mainappid", appid, MUSIC_APP, "\"com.samsung.music-player\" should be returned\n");
}

static void utc_livebox_service_get_pkglist_by_pkgid_n(void)
{
	int ret;
	ret = livebox_service_get_pkglist_by_pkgid(NULL, list_cb, NULL);
	dts_check_eq("livebox_service_get_pkglist_by_pkgid", ret, LB_STATUS_ERROR_INVALID, "INVALID should be returned");
}

static void utc_livebox_service_get_pkglist_by_pkgid_p(void)
{
	int ret;
	ret = livebox_service_get_pkglist_by_pkgid(MUSIC_APP, list_cb, NULL);
	dts_check_ge("livebox_service_get_pkglist_by_pkgid", ret, 0, "greater than or equal to 0");
}

static void utc_livebox_service_pkgname_n(void)
{
	char *pkgname;
	pkgname = livebox_service_pkgname(NULL);
	dts_check_eq("livebox_service_pkgname", pkgname, NULL, "lbid is not exists");
}

static void utc_livebox_service_pkgname_p(void)
{
	char *pkgname;
	pkgname = livebox_service_pkgname(MUSIC_APP);
	dts_check_str_eq("livebox_service_pkgname", pkgname, MUSIC_LIVEBOX, "livebox music-player");
}

static void utc_livebox_service_is_primary_n(void)
{
	int ret;
	ret = livebox_service_is_primary(NULL);
	dts_check_str_eq("livebox_service_is_primary", ret, 0, "\"NULL\" is not primary(0)\n");
}

static void utc_livebox_service_is_primary_p(void)
{
	int ret;
	ret = livebox_service_is_primary(MUSIC_LIVEBOX);
	dts_check_str_eq("livebox_service_is_primary", ret, 1, "\"com.samsung.music-player\" is primary(1)\n");
}

static void utc_livebox_service_provider_name_n(void)
{
//static char *livebox_service_provider_name(const char *lbid);
	char *provider;
	provider = livebox_service_provider_name(NULL);
	dts_check_eq("livebox_service_provider_name", provider, NULL, "returns NULL");
}

static void utc_livebox_service_provider_name_p(void)
{
	char *provider;
	provider = livebox_service_provider_name(MUSIC_LIVEBOX);
	dts_check_str_eq("livebox_service_provider_name", provider, MUSIC_LIVEBOX, "inhouse livebox");
}

static void utc_livebox_service_setup_appid_n(void)
{
//static char *livebox_service_setup_appid(const char *lbid);
	char *setup;
	setup = livebox_service_setup_appid(NULL);
	dts_check_eq("livebox_service_setup_appid", setup, NULL, "setup is NULL");
}

static void utc_livebox_service_setup_appid_p(void)
{
	char *setup;
	setup = livebox_service_setup_appid(MUSIC_LIVEBOX);
	dts_check_eq("livebox_service_setup_appid", setup, MUSIC_LIVEBOX, "setup app");
}

static void utc_livebox_service_appid_n(void)
{
	char *appid;
	appid = livebox_service_appid(NULL);
	dts_check_eq("livebox_service_appid", appid, NULL, "appid is NULL");
}

static void utc_livebox_service_appid_p(void)
{
	char *appid;
	appid = livebox_service_appid(MUSIC_LIVEBOX);
	dts_check_str_eq("livebox_service_appid", appid, MUSIC_APP, "appid is com.samsung.music-player");
}

static void utc_livebox_service_i18n_name_n(void)
{
	char *name;
	name = livebox_service_i18n_name(NULL, NULL);
	dts_check_eq("livebox_service_i18n_name", name, NULL, "name is NULL");
}

static void utc_livebox_service_i18n_name_p(void)
{
	char *name;
	name = livebox_service_i18n_name(MUSIC_LIVEBOX, NULL);
	dts_check_ne("livebox_service_i18n_name", name, NULL, "name is not NULL");
}

static void utc_livebox_service_privew_n(void)
{
	char *preview;
	preview = livebox_service_preview(MUSIC_LIVEBOX, LB_SIZE_TYPE_1x1);
	dts_check_eq("livebox_service_preview", preview, NULL, "preview is NULL");
}

static void utc_livebox_service_preview_p(void)
{
	char *preview;
	preview = livebox_service_preview(MUSIC_LIVEBOX, LB_SIZE_TYPE_1x1);
	dts_check_ne("livebox_service_preview", preview, NULL, "preview is not NULL");
}

static void utc_livebox_service_content_n(void)
{
	char *content;
	content = livebox_service_content(NULL);
	dts_check_eq("livebox_service_content", content, NULL, "content is NULL");
}

static void utc_livebox_service_content_p(void)
{
	char *content;
	content = livebox_service_content(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_content", content, NULL, "content is not NULL");
}

static void utc_livebox_service_i18n_icon_p(void)
{
	char *icon;

	icon = livebox_service_i18n_icon(NULL, NULL);
	dts_check_eq("livebox_service_i18n_icon", icon, NULL, "icon is NULL");
}

static void utc_livebox_service_i18n_icon_n(void)
{
	char *icon;

	icon = livebox_service_i18n_icon(MUSIC_LIVEBOX, NULL);
	dts_check_ne("livebox_service_i18n_icon", icon, NULL, "icon is not NULL");
}

static void utc_livebox_service_libexec_p(void)
{
	char *libexec;
	libexec = livebox_service_libexec(NULL);
	dts_check_eq("livebox_service_libexec", libexec, NULL, "libexec is NULL");
}

static void utc_livebox_service_libexec_n(void)
{
	char *libexec;
	libexec = livebox_service_libexec(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_libexec", libexec, NULL, "libexec is not NULL");
}

static void utc_livebox_service_nodisplay_p(void)
{
	int ret;
	ret = livebox_service_nodisplay(NULL);
	dts_check_eq("livebox_service_nodisplay", ret, 0, "nodisplay is false");
}

static void utc_livebox_service_nodisplay_n(void)
{
	int ret;
	ret = livebox_service_nodisplay(MUSIC_LIVEBOX);
	dts_check_eq("livebox_service_nodisplay", ret, 0, "nodisplay is false");
}

static void utc_livebox_service_abi_p(void)
{
	char *abi;
	abi = livebox_service_abi(NULL);
	dts_check_eq("livebox_service_abi", abi, NULL, "abi is NULL");
}

static void utc_livebox_service_abi_n(void)
{
	char *abi;
	abi = livebox_service_abi(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_abi", abi, NULL, "abi is not NULL");
}

static void utc_livebox_service_is_enabled_n(void)
{
	/*!
	 * static int livebox_service_is_enabled(const char *lbid);
	 * - Not implemented
	 */
}

static void utc_livebox_service_is_enabled_p(void)
{
	/*!
	 * static int livebox_service_is_enabled(const char *lbid);
	 * - Not implemented
	 */
}

static void utc_livebox_service_lb_script_path_n(void)
{
	char *lb_path;
	lb_path = livebox_service_lb_script_path(NULL);
	dts_check_eq("livebox_service_lb_script_path", lb_path, NULL, "lb_path is NULL");
}

static void utc_livebox_service_lb_script_path_p(void)
{
	char *lb_path;
	lb_path = livebox_service_lb_script_path(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_lb_script_path", lb_path, NULL, "lb_path is NULL");
}

static void utc_livebox_service_lb_script_group_n(void)
{
	char *lb_group;
	lb_group = livebox_service_lb_script_group(NULL);
	dts_check_ne("livebox_service_lb_script_group", lb_group, NULL, "lb_group is NULL");
}

static void utc_livebox_service_lb_script_group_p(void)
{
	char *lb_group;
	lb_group = livebox_service_lb_script_group(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_lb_script_group", lb_group, NULL, "lb_group is NULL");
}

static void utc_livebox_service_pd_script_path_n(void)
{
	char *pd_script;
	pd_script = livebox_service_pd_script_path(NULL);
	dts_check_eq("livebox_service_pd_script_path", pd_script, NULL, "pd_script is NULL");
}

static void utc_livebox_service_pd_script_path_p(void)
{
	char *pd_script;
	pd_script = livebox_service_pd_script_path(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_pd_script_path", pd_script, NULL, "pd_script is NULL");
}

static void utc_livebox_service_pd_script_group_p(void)
{
	char *pd_group;
	pd_group = livebox_service_pd_script_group(MUSIC_LIVEBOX);
	dts_check_ne("livebox_service_pd_script_group", pd_group, NULL, "pd_script is NULL");
}

static void utc_livebox_service_pd_script_group_n(void)
{
	char *pd_group;
	pd_group = livebox_service_pd_script_group(MUSIC_LIVEBOX);
	dts_check_eq("livebox_service_pd_script_group", pd_group, NULL, "pd_script is NULL");
}

static void utc_livebox_service_get_supported_sizes_n(void)
{
	int cnt = NR_OF_SIZE_LIST;
	int w[NR_OF_SIZE_LIST];
	int h[NR_OF_SIZE_LIST];
	int ret;

	ret = livebox_service_get_supported_sizes(NULL, &cnt, w, h);
	dts_check_eq("livebox_service_get_supported_sizes", ret, LB_STATUS_ERROR_INVALID, "supported sizes is 0");
}

static void utc_livebox_service_get_supported_sizes_p(void)
{
	int cnt = NR_OF_SIZE_LIST;
	int w[NR_OF_SIZE_LIST];
	int h[NR_OF_SIZE_LIST];
	int ret;

	ret = livebox_service_get_supported_sizes(MUSIC_LIVEBOX, &cnt, w, h);
	dts_check_eq("livebox_service_get_supported_sizes", ret, 0, "supported sizes is 0");
}

static void utc_livebox_service_get_supported_size_types_n(void)
{
	int cnt = NR_OF_SIZE_LIST;
	int sizes[NR_OF_SIZE_LIST];
	int ret;

	ret = livebox_service_get_supported_size_types(NULL, &cnt, sizes);
	dts_check_eq("livebox_service_get_supported_sizes", ret, LB_STATUS_ERROR_INVALID, "supported sizes is 0");
}

static void utc_livebox_service_get_supported_size_types_p(void)
{
	int cnt = NR_OF_SIZE_LIST;
	int sizes[NR_OF_SIZE_LIST];
	int ret;

	ret = livebox_service_get_supported_size_types(MUSIC_LIVEBOX, &cnt, sizes);
	dts_check_eq("livebox_service_get_supported_sizes", ret, 0, "supported sizes is 0");
}

static int category_cb(const char *cluster, const char *category, void *data)
{
	return 0;
}

static void utc_livebox_service_enumerate_category_list_n(void)
{
	int ret;
	ret = livebox_service_enumerate_category_list(NULL, NULL, NULL);
	dts_check_eq("livebox_service_enumerate_category_list", ret, LB_STATUS_ERROR_INVALID, "invalid argument");
}

static void utc_livebox_service_enumerate_category_list_p(void)
{
	int ret;
	ret = livebox_service_enumerate_category_list("default", category_cb, NULL);
	dts_check_eq("livebox_service_enumerate_category_list", ret, LB_STATUS_ERROR_INVALID, "invalid argument");
}

static void utc_livebox_service_enumerate_cluster_list_n(void)
{
//static int livebox_service_enumerate_cluster_list(int (*cb)(const char *cluster, void *data), void *data);
	int ret;
	ret = livebox_service_enumerate_cluster_list(NULL, NULL);
	dts_check_eq("livebox_service_enumerate_cluster_list", ret, LB_STATUS_ERROR_INVALID, "invalid argument");
}

static void utc_livebox_service_enumerate_cluster_list_p(void)
{
	int ret;
	ret = livebox_service_enumerate_cluster_list(cluster_cb, NULL);
	dts_check_ne("livebox_service_enumerate_cluster_list", ret, 0, "invalid argument");
}

static void utc_livebox_service_init_n(void)
{
	/*!
	 * \note
	 * Unable to test initialize function
	 */
}

static void utc_livebox_service_init_p(void)
{
	int ret;

	ret = livebox_service_init();
	dts_check_eq("livebox_service_init", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_service_fini_n(void)
{
	/*!
	 * \note
	 * Unable to test finalize function
	 */
}

static void utc_livebox_service_fini_p(void)
{
	int ret;
	ret = livebox_service_fini();
	dts_check_eq("livebox_service_fini", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_service_pkglist_create_n(void)
{
	/*!
	 * \note
	 * Unable to test finalize function
	 */
}

static void utc_livebox_service_pkglist_create_p(void)
{
	struct pkglist_handle *handle;

	handle = livebox_service_pkglist_create(NULL, NULL);
	dts_check_ne("livebox_service_pkglist_create", handle, NULL, "Success");
}

static void utc_livebox_service_get_pkglist_item_n(void)
{
	char *lbid;
	char *pkgname;
	int prime;
	int ret;

	ret = livebox_service_get_pkglist_item(NULL, &lbid, &pkgname, &prime);
	dts_check_eq("livebox_service_get_pkglist_item", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_service_get_pkglist_item_p(void)
{
	struct pkglist_handle *handle;
	char *lbid;
	char *pkgname;
	int prime;
	int ret;

	handle = livebox_service_pkglist_create(NULL, NULL);
	ret = livebox_service_get_pkglist_item(handle, &lbid, &pkgname, &prime);
	dts_check_eq("livebox_service_get_pkglist_item", ret, LB_STATUS_SUCCESS, "Success");
}

static void utc_livebox_service_pkglist_destroy_n(void)
{
	int ret;

	ret = livebox_service_pkglist_destroy(NULL);
	dts_check_eq("livebox_service_pkglist_destroy", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

static void utc_livebox_service_pkglist_destroy_p(void)
{
	struct pkglist_handle *handle;
	int ret;

	handle = livebox_service_pkglist_create(NULL, NULL);
	ret = livebox_service_pkglist_destroy(handle);
	dts_check_eq("livebox_service_pkglist_destroy", ret, LB_STATUS_ERROR_INVALID, "Invalid");
}

struct tet_testlist tet_testlist[] = {
	{ utc_livebox_service_get_size_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_size_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_size_type_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_size_type_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_mouse_event_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_mouse_event_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_touch_effect_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_touch_effect_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_need_frame_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_need_frame_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_trigger_update_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_trigger_update_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_trigger_update_with_content_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_trigger_update_with_content_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_change_period_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_change_period_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_get_applist_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_applist_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_mainappid_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_mainappid_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_by_pkgid_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_by_pkgid_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pkgname_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_pkgname_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_is_primary_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_is_primary_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_provider_name_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_provider_name_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_setup_appid_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_setup_appid_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_appid_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_appid_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_i18n_name_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_i18n_name_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_privew_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_preview_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_content_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_content_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_content_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_i18n_icon_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_i18n_icon_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_libexec_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_libexec_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_nodisplay_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_nodisplay_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_abi_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_abi_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_is_enabled_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_is_enabled_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_lb_script_path_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_lb_script_path_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_lb_script_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_lb_script_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pd_script_path_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_pd_script_path_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pd_script_group_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pd_script_group_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_supported_sizes_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_supported_sizes_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_get_supported_size_types_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_supported_size_types_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_enumerate_category_list_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_enumerate_category_list_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_enumerate_cluster_list_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_enumerate_cluster_list_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_init_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_init_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_fini_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_fini_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pkglist_create_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_pkglist_create_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_item_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_get_pkglist_item_p, POSITIVE_TC_IDX },
	{ utc_livebox_service_pkglist_destroy_n, NEGATIVE_TC_IDX },
	{ utc_livebox_service_pkglist_destroy_p, POSITIVE_TC_IDX },
	{ NULL, 0 },
};

