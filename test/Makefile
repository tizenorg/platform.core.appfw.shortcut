all:
	@gcc homescreen.c -Wall -o homescreen `pkg-config ecore elementary shortcut --cflags --libs`
	@gcc application.c -Wall -o application `pkg-config ecore elementary shortcut --cflags --libs`
	@gcc shortcut.c -Wall -o shortcut `pkg-config ecore elementary shortcut --cflags --libs`
