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

