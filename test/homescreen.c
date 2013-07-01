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

int shortcut_request_cb(const char *pkgname, const char *name, int type, const char *exec, const char *icon, int pid, double period, void *data)
{
	printf("SERVER: name: %s, type: %d, exec: %s, icon: %s, pid: %d, data: %p, period: %lf\n",
		name, type, exec, icon, pid, data, period);
	return 0;
}

int elm_main(int argc, char *argv[])
{
	shortcut_set_request_cb(shortcut_request_cb, NULL);

	elm_run();
	elm_shutdown();

	return 0;
}

ELM_MAIN()
/* End of a file */
