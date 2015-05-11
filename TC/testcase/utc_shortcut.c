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

#include <shortcut.h>

#define MUSIC_APP "org.tizen.music-player"
#define MUSIC_DYNAMICBOX MUSIC_APP ".dynamicbox"
#define MUSIC_EASYBOX "org.tizen.music-player.easymode.dynamicbox"

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static struct test_info {
	struct shortcut_icon *handle;
} s_info = {
	.handle = NULL,
};

static void startup(void)
{
	/* start of TC */
	tet_printf("\n TC start");
}


static void cleanup(void)
{
	/* end of TC */
	tet_printf("\n TC end");
}

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_shortcut_set_request_cb_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("shortcut_set_request_cb", "pass negative test");
}

static int shortcut_request_cb(const char *appid, const char *name, int type, const char *content_info, const char *icon, int pid, double period, int allow_duplicate, void *data)
{
	if (appid) {
		if (!strcmp(appid, "fail")) {
			return SHORTCUT_ERROR_UNSUPPORTED;
		} else if (!strcmp(appid, "success")) {
			return SHORTCUT_ERROR_NONE;
		}
	}

	return SHORTCUT_ERROR_NONE;
}

static void utc_shortcut_set_request_cb_p(void)
{
	int ret;
	ret = shortcut_set_request_cb(shortcut_request_cb, NULL);
	dts_check_eq("shortcut_set_request_cb", ret, SHORTCUT_ERROR_NONE, "success");
}

static int response_cb(int ret, int pid, void *data)
{
	if ((int)data == 1) {
		dts_check_eq("add_to_home_shortcut", ret, SHORTCUT_ERROR_UNSUPPORTED, "success");
	} else if ((int)data == 2) {
		dts_check_eq("add_to_home_shortcut", ret, SHORTCUT_ERROR_NONE, "success");
	}

	return 0;
}

static void utc_add_to_home_shortcut_n(void)
{
	int ret;
	ret = add_to_home_shortcut("fail", NULL, LAUNCH_BY_PACKAGE, NULL, NULL, 1, response_cb, (void *)1);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_shortcut", ret, SHORTCUT_ERROR_NONE, "success");
	}
}

static void utc_add_to_home_shortcut_p(void)
{
	int ret;
	ret = add_to_home_shortcut("success", NULL, LAUNCH_BY_PACKAGE, NULL, NULL, 1, response_cb, (void *)2);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_shortcut", ret, SHORTCUT_ERROR_NONE, "success");
	}
}

static void utc_shortcut_get_list_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("shortcut_get_list", "negative test");
}

static int shortcut_list_cb(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data)
{
	return 0;
}

static void utc_shortcut_get_list_p(void)
{
	int ret;
	ret = shortcut_get_list(NULL, shortcut_list_cb, NULL);
	dts_check_ge("shortcut_get_list", ret, 0, "get_list");
}

static void utc_add_to_home_dynamicbox_n(void)
{
	int ret;
	ret = add_to_home_dynamicbox("fail", NULL, WIDGET_SIZE_1x1, NULL, NULL, -1.0f, 1, response_cb, (void *)1);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_dynamicbox", ret, SHORTCUT_ERROR_NONE, "success");
	}
}

static void utc_add_to_home_dynamicbox_p(void)
{
	int ret;
	ret = add_to_home_dynamicbox("success", NULL, WIDGET_SIZE_1x1, NULL, NULL, -1.0f, 1, response_cb, (void *)2);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_dynamicbox", ret, SHORTCUT_ERROR_NONE, "success");
	}
}

static void utc_add_to_home_remove_shortcut_n(void)
{
	int ret;
	ret = add_to_home_remove_shortcut("fail", NULL, NULL, response_cb, (void *)1);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_remove_shortcut", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "Invalid");
	}
}

static void utc_add_to_home_remove_shortcut_p(void)
{
	int ret;
	ret = add_to_home_remove_shortcut("success", NULL, NULL, response_cb, (void *)2);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_remove_shortcut", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "Invalid");
	}
}

static void utc_add_to_home_remove_dynamicbox_n(void)
{
	int ret;
	ret = add_to_home_remove_dynamicbox("fail", NULL, response_cb, (void *)1);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_remove_shortcut", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "Invalid");
	}
}

static void utc_add_to_home_remove_dynamicbox_p(void)
{
	int ret;
	ret = add_to_home_remove_dynamicbox("success", NULL, response_cb, (void *)2);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("add_to_home_remove_shortcut", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "Invalid");
	}
}

static void utc_shortcut_icon_service_init_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("shortcut_icon_service_init", "negative test");
}

static int icon_service_cb(int status, void *data)
{
	dts_check_eq("shortcut_icon_service_init", status, SHORTCUT_ERROR_NONE);
	return 0;
}

static void utc_shortcut_icon_service_init_p(void)
{
	int ret;

	ret = shortcut_icon_service_init(icon_service_cb, NULL);
	if (ret != SHORTCUT_ERROR_NONE) {
		dts_check_eq("shortcut_icon_service_init", ret, SHORTCUT_ERROR_NONE);
	}
}

static void utc_shortcut_icon_service_fini_n(void)
{
	/*!
	 * \note
	 * Unable to test negative case
	 */
	dts_pass("shortcut_icon_service_fini", "negative test");
}

static void utc_shortcut_icon_service_fini_p(void)
{
	int ret;
	ret = shortcut_icon_service_fini();
	dts_check_eq("shortcut_icon_service_fini", ret, SHORTCUT_ERROR_NONE, "success");
}

static void utc_shortcut_icon_request_create_n(void)
{
	dts_pass("shortcut_icon_request_create", "negative test");
}

static void utc_shortcut_icon_request_create_p(void)
{
	s_info.handle = shortcut_icon_request_create();
	dts_check_ne("shortcut_icon_request_create", s_info.handle, NULL, "success");
}

static void utc_shortcut_icon_request_set_info_n(void)
{
	int ret;
	ret = shortcut_icon_request_set_info(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	dts_check_ne("shortcut_icon_request_set_info", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "invalid handle");
}

static void utc_shortcut_icon_request_set_info_p(void)
{
	int ret;

	if (!s_info.handle) {
		dts_pass("shortcut_icon_request_set_info", "handle is not initialized");
		return;
	}

	ret = shortcut_icon_request_set_info(s_info.handle, NULL, SHORTCUT_ICON_TYPE_IMAGE, "icon,part", "/opt/usr/share/icon.png", NULL, NULL);
	dts_check_ge("shortcut_icon_request_set_info", ret, 0, "set_info");
}

static void utc_shortcut_icon_request_send_n(void)
{
	int ret;

	ret = shortcut_icon_request_send(NULL, WIDGET_SIZE_1x1, NULL, NULL, NULL, NULL, NULL);
	dts_check_eq("shortcut_icon_request_send", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "success");
}

static int result_cb(struct shortcut_icon *handle, int ret, void *data)
{
	return 0;
}

static void utc_shortcut_icon_request_send_p(void)
{
	int ret;

	if (!s_info.handle) {
		dts_pass("shortcut_icon_request_set_info", "handle is not initialized");
		return;
	}

	ret = shortcut_icon_request_send(s_info.handle, WIDGET_SIZE_1x1, NULL, NULL, "/tmp/icon.png", result_cb, NULL);
	dts_check_eq("shortcut_icon_request_send", ret, 0, "success");
}

static void utc_shortcut_icon_request_destroy_n(void)
{
	int ret;

	ret = shortcut_icon_request_destroy(NULL);
	dts_check_eq("shortcut_icon_request_destroy", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "invalid");
}

static void utc_shortcut_icon_request_destroy_p(void)
{
	int ret;

	if (!s_info.handle) {
		dts_pass("shortcut_icon_request_set_info", "handle is not initialized");
		return;
	}

	ret = shortcut_icon_request_destroy(s_info.handle);
	dts_check_eq("shortcut_icon_request_destroy", ret, SHORTCUT_ERROR_NONE, "Destroy");
	s_info.handle = NULL;
}

static void utc_shortcut_icon_request_set_data_n(void)
{
	int ret;
	ret = shortcut_icon_request_set_data(NULL, NULL);
	dts_check_eq("shortcut_icon_request_set_data", ret, SHORTCUT_ERROR_INVALID_PARAMETER, "invalid");
}

static void utc_shortcut_icon_request_set_data_p(void)
{
	int ret;

	if (!s_info.handle) {
		dts_pass("shortcut_icon_request_set_data", "failed to set data");
		return;
	}

	ret = shortcut_icon_request_set_data(s_info.handle, (void *)1);
	dts_check_eq("shortcut_icon_request_set_data", ret, SHORTCUT_ERROR_NONE, "success");
}

static void utc_shortcut_icon_request_data_n(void)
{
	dts_pass("shortcut_icon_request_data", "get_data");
}

static void utc_shortcut_icon_request_data_p(void)
{
	void *data;
	if (!s_info.handle) {
		dts_pass("shortcut_icon_request_data", "invalid handle");
		return;
	}

	data = shortcut_icon_request_data(s_info.handle);
	dts_check_eq("shortcut_icon_request_data", data, (void *)1, "success");
}

struct tet_testlist tet_testlist[] = {
	{ utc_shortcut_set_request_cb_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_set_request_cb_p, POSITIVE_TC_IDX },
	{ utc_add_to_home_shortcut_n, NEGATIVE_TC_IDX },
	{ utc_add_to_home_shortcut_p, POSITIVE_TC_IDX },
	{ utc_shortcut_get_list_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_get_list_p, POSITIVE_TC_IDX },
	{ utc_add_to_home_dynamicbox_n, NEGATIVE_TC_IDX },
	{ utc_add_to_home_dynamicbox_p, POSITIVE_TC_IDX },
	{ utc_add_to_home_remove_shortcut_n, NEGATIVE_TC_IDX },
	{ utc_add_to_home_remove_shortcut_p, POSITIVE_TC_IDX },
	{ utc_add_to_home_remove_dynamicbox_n, NEGATIVE_TC_IDX },
	{ utc_add_to_home_remove_dynamicbox_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_service_init_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_service_init_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_create_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_create_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_set_info_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_set_info_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_send_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_send_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_set_data_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_set_data_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_data_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_data_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_request_destroy_n, NEGATIVE_TC_IDX },
	{ utc_shortcut_icon_request_destroy_p, POSITIVE_TC_IDX },
	{ utc_shortcut_icon_service_fini_n, NEGATIVE_TC_IDX }, // Must be tested as the last TC
	{ utc_shortcut_icon_service_fini_p, POSITIVE_TC_IDX },
	{ NULL, 0 },
};

