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

static int result_cb(int ret, int pid, void *data)
{
	printf("Client: Return %d (%d)\n", ret, pid);
	//elm_exit();
	return 0;
}

static Eina_Bool shortcut_add_cb(void *data)
{
	int ret;

	ret = add_to_home_shortcut("pkgname", "MyName", 0, "/usr/bin/true", "/opt/share/image/what.png", result_cb, NULL);
	printf("Client: shortcut_add_to_home returns: %d\n", ret);

	ret = add_to_home_dynamicbox("pkgname", "MyName", 0, "/usr/bin/true", "/opt/share/image/what.png", 1.0f, result_cb, NULL);
	printf("Client: shortcut_add_to_home_with_period returns: %d\n", ret);

	return ECORE_CALLBACK_RENEW;
}

int elm_main(int argc, char *argv[])
{
	Ecore_Timer *timer;

	timer = ecore_timer_add(3.0f, shortcut_add_cb, NULL);
	if (!timer) {
		printf("Failed to add a timer\n");
	}

	elm_run();
	elm_shutdown();

	return 0;
}

ELM_MAIN()
/* End of a file */
