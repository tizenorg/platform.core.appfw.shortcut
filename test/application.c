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
