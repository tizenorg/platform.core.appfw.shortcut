/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <shortcut.h>

int shortcut_request_cb(const char *name, int type, const char *exec, const char *icon, int pid, void *data)
{
	printf("SERVER: name: %s, type: %d, exec: %s, icon: %s, pid: %d, data: %p\n",
		name, type, exec, icon, pid, data);
	return 0;
}

int elm_main(int argc, char *argv[])
{
	int ret;
	shortcut_set_request_cb(shortcut_request_cb, NULL);

	elm_run();
	elm_shutdown();

	return 0;
}

ELM_MAIN()
/* End of a file */

