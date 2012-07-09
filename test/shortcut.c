/*
 * libshortcut
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>, Youngjoo Park <yjoo93.park@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software, either express or implied, including but not limited to the implied warranties of merchantability, fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using, modifying or distributing this software or its derivatives.
 *
 *
 */

#include <Elementary.h>
#include <shortcut.h>

static int shortcut_list_cb(const char *pkgname, const char *name, const char *param, void *data)
{
	printf("pkgname[%s] name[%s] param[%s]\n", pkgname, name, param);
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
