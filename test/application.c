/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved 
 * PROPRIETARY/CONFIDENTIAL
 * This software is the confidential and proprietary information of 
 * SAMSUNG ELECTRONICS ("Confidential Information"). You agree and acknowledge that 
 * this software is owned by Samsung and you 
 * shall not disclose such Confidential Information and shall 
 * use it only in accordance with the terms of the license agreement 
 * you entered into with SAMSUNG ELECTRONICS.  SAMSUNG make no 
 * representations or warranties about the suitability 
 * of the software, either express or implied, including but not 
 * limited to the implied warranties of merchantability, fitness for 
 * a particular purpose, or non-infringement. 
 * SAMSUNG shall not be liable for any damages suffered by licensee arising out of or 
 * related to this software.
 * */

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

	ret = shortcut_add_to_home("MyName", 0, "/usr/bin/true", "/opt/share/image/what.png", result_cb, NULL);
	printf("Client: shortcut_add_to_home returns: %d\n", ret);

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
