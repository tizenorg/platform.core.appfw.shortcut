/*
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <Elementary.h>
#include <shortcut.h>

static int result_cb(struct shortcut_icon *handle, int ret, void *data)
{
	printf("Client: Return %d (%p)\n", ret, handle);
	return 0;
}

static Eina_Bool test_main(void *data)
{
	struct shortcut_icon *handle;
	static int idx = 0;
	int ret;
	char filename[256];
	int type;

	idx++;

	handle = shortcut_icon_request_create();
	if (!handle) {
		printf("Failed to create a request\n");
		return ECORE_CALLBACK_RENEW;
	}

	printf("Test: %d\n", idx);
	ret = shortcut_icon_request_set_info(handle, NULL, SHORTCUT_ICON_TYPE_IMAGE, DEFAULT_ICON_PART, "/usr/share/icons/default/small/org.tizen.music-player.png", NULL, NULL);
	printf("NAME set_info: %d\n", ret);

	snprintf(filename, sizeof(filename), "App Name %d", idx);
	ret = shortcut_icon_request_set_info(handle, NULL, SHORTCUT_ICON_TYPE_TEXT, DEFAULT_NAME_PART, filename, NULL, NULL);
	printf("TEXT set_info: %d\n", ret);

	snprintf(filename, sizeof(filename), "/opt/usr/share/live_magazine/always/out%d.png", idx);

	switch (idx % 7) {
	case 0: type = DYNAMICBOX_TYPE_1x1; break;
	case 1: type = DYNAMICBOX_TYPE_2x1; break;
	case 2: type = DYNAMICBOX_TYPE_2x2; break;
	case 3: type = DYNAMICBOX_TYPE_4x1; break;
	case 4: type = DYNAMICBOX_TYPE_4x2; break;
	case 5: type = DYNAMICBOX_TYPE_4x3; break;
	case 6: type = DYNAMICBOX_TYPE_4x4; break;
	default: type = DYNAMICBOX_TYPE_1x1; break;
	}

	ret = shortcut_icon_request_send(handle, type, NULL, NULL, filename, result_cb, NULL);
	printf("request: %d\n", ret);

	ret = shortcut_icon_request_destroy(handle);
	printf("destroy: %d\n", ret);
	return ECORE_CALLBACK_RENEW;
}

static int initialized_cb(int status, void *data)
{
	printf("Hello initializer\n");
	return 0;
}

int elm_main(int argc, char *argv[])
{
	shortcut_icon_service_init(NULL, NULL);

	ecore_timer_add(5.0f, test_main, NULL);

	elm_run();
	shortcut_icon_service_fini();
	elm_shutdown();
	return 0;
}

ELM_MAIN()
/* End of a file */
