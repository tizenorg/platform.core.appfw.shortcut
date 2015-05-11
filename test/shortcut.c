/*
 * Copyright (c) 2011 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

static int shortcut_list_cb(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data)
{
	printf("appid[%s] icon[%s], name[%s] extra_key[%s], extra_ata[%s]\n", appid, icon, name, extra_key, extra_data);
	return 0;
}

int elm_main(int argc, char *argv[])
{
	int ret;
	ret = shortcut_get_list(NULL, shortcut_list_cb, NULL);
	if (ret < 0)
		printf("Error: %d\n", ret);

	elm_run();
	elm_shutdown();

	return 0;
}

ELM_MAIN()
/* End of a file */
