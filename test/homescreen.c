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

