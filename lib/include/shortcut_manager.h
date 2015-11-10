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

#ifndef __SHORTCUT_MANAGER_H__
#define __SHORTCUT_MANAGER_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file shortcut_manager.h
 * @brief This file declares the API of the libshortcut library.
 */

/**
 * @addtogroup SHORTCUT_MODULE
 * @{
 */

/**
 * @brief Enumeration for shortcut types.
 * @details Basically, two types of shortcuts are defined.
 *          Every homescreen developer should support these types of shortcuts.
 *          Or return a proper errno to figure out why the application failed to add a shortcut.
 *          #LAUNCH_BY_APP is used for adding a package itself as a shortcut.
 *          #LAUNCH_BY_URI is used for adding a shortcut for "uri" data.
 * @since_tizen 2.3
 */
typedef enum _shortcut_type {
	/**< Use these */
	LAUNCH_BY_APP	= 0x00000000,	/**< Launch the application itself */
	LAUNCH_BY_URI	= 0x00000001,	/**< Launch the application with the given data(URI) */
} shortcut_type;

/**
 * @brief Enumeration for values of shortcut response types.
 * @since_tizen 2.3
 */
enum shortcut_error_e {
	SHORTCUT_ERROR_NONE = TIZEN_ERROR_NONE,				/**< Successfully handled */
	SHORTCUT_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,	/**< Shortcut request is not valid, invalid parameter or invalid argument value */
	SHORTCUT_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,	/**< Memory is not enough to handle a new request */
	SHORTCUT_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR,		/**< Unable to access the file or DB. Check your resource files */
	SHORTCUT_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,	/**< Has no permission to add a shortcut */
	SHORTCUT_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NOT_SUPPORTED,	/**< Not supported shortcut */
	SHORTCUT_ERROR_RESOURCE_BUSY = TIZEN_ERROR_RESOURCE_BUSY,		/**< Receiver is busy, try again later */
	SHORTCUT_ERROR_NO_SPACE = TIZEN_ERROR_SHORTCUT | 0x0001,	/**< There is no space to add a new shortcut */
	SHORTCUT_ERROR_EXIST = TIZEN_ERROR_SHORTCUT | 0x0002,		/**< Shortcut is already added */
	SHORTCUT_ERROR_FAULT = TIZEN_ERROR_SHORTCUT | 0x0004,		/**< Failed to add a shortcut. Unrecoverable error */
	SHORTCUT_ERROR_COMM = TIZEN_ERROR_SHORTCUT | 0x0040		/**< Connection is not established. or there is a problem in the communication */
};

/**
 * @brief Enumeration for sizes of shortcut widget.
 * @since_tizen 2.4
 */
typedef enum shortcut_widget_size {
	WIDGET_SIZE_DEFAULT      = 0x10000000,	/* Type mask for the normal mode widget , don't use this value for specific size.*/
	WIDGET_SIZE_1x1	         = 0x10010000,	/**< 1x1 */
	WIDGET_SIZE_2x1	         = 0x10020000,	/**< 2x1 */
	WIDGET_SIZE_2x2	         = 0x10040000,	/**< 2x2 */
	WIDGET_SIZE_4x1	         = 0x10080000,	/**< 4x1 */
	WIDGET_SIZE_4x2	         = 0x10100000,	/**< 4x2 */
	WIDGET_SIZE_4x3          = 0x10200000,	/**< 4x3 */
	WIDGET_SIZE_4x4          = 0x10400000,	/**< 4x4 */
	WIDGET_SIZE_4x5          = 0x11000000,	/**< 4x5 */
	WIDGET_SIZE_4x6          = 0x12000000,  /**< 4x6 */
	WIDGET_SIZE_EASY_DEFAULT = 0x30000000,	/* Type mask for the easy mode widget, don't use this value for specific size. */
	WIDGET_SIZE_EASY_1x1     = 0x30010000,	/**< Easy mode 1x1 */
	WIDGET_SIZE_EASY_3x1     = 0x30020000,	/**< Easy mode 3x2 */
	WIDGET_SIZE_EASY_3x3     = 0x30040000,	/**< Easy mode 3x3 */
} shortcut_widget_size_e;

/**
 * @brief Called to receive the result of shortcut_add_to_home().
 * @since_tizen 2.3
 * @param[in] ret The result value, it could be @c 0 if it succeeds to add a shortcut,
 *                otherwise it returns an errno
 * @param[in] data The callback data
 * @return int @c 0 if there is no error,
               otherwise errno
 * @see shortcut_add_to_home()
 */
typedef int (*result_cb_t)(int ret, void *data);

/**
 * @brief Adds a shortcut to home, asynchronously
 * @remarks If a homescreen does not support this feature, you will get a proper error code.\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 * @param[in] name The name of the created shortcut icon
 * @param[in] type The type of shortcuts
 * @param[in] uri The specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon The absolute path of an icon file
 * @param[in] allow_duplicate @c 1 if it accepts the duplicated shortcut,
 *                            otherwise @c 0
 * @param[in] result_cb The address of the callback function that is called when the result comes back from the viewer
 * @param[in] data The callback data that is used in the callback function
 *
 * @return #SHORTCUT_ERROR_NONE on success, other value on failure
 * @retval #SHORTCUT_ERROR_NONE Successful
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SHORTCUT_ERROR_IO_ERROR I/O error
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SHORTCUT_ERROR_NOT_SUPPORTED Not supported
 * @retval #SHORTCUT_ERROR_RESOURCE_BUSY Device or resource busy
 * @retval #SHORTCUT_ERROR_NO_SPACE No space
 * @retval #SHORTCUT_ERROR_EXIST Already exist
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_COMM Connection failed
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 * @see result_cb_t
 *
 * @par Example
 * @code
 *
 * #include <stdio.h>
 * #include <shortcut_manager.h>
 *
 * static int result_cb(int ret, int pid, void *data)
 * {
 * 	if (ret < 0)
 * 		dlog_print("Failed to add a shortcut: %s\n", perror(ret));
 *
 *	dlog_print("Processed by the %d\n", pid);
 * 	return 0;
 * }
 *
 * static int app_create(void *data)
 * {
 *	char* data_path = app_get_data_path();
 *	int path_len = strlen(data_path)+10;
 *	char * path = malloc(path_len);
 *	memset(path, 0, path_len);
 *	strncat(path, data_path, path_len);
 *	strncat(path, "Friend.jpg", path_len); 
 *
 * 	shortcut_add_to_home("With friends",
 * 					LAUNCH_BY_URI, "gallery:0000-0000",
 * 					path, 0, result_cb, NULL);
 *	free(path);
 *
 * 	return 0;
 * }
 *
 * @endcode
 */
extern int shortcut_add_to_home(const char *name, shortcut_type type, const char *uri, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data);

/**
 * @brief Adds a widget to home, asynchronously.
 * @remarks If a homescreen does not support this feature, you will get a proper error code.\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @param[in] name The name of the created widget. Will be shown when the widget is not prepared.
 * @param[in] size_type The size of widget
 * @param[in] widget_id Widget id
 * @param[in] icon The absolute path of an icon file. Will be shown when the widget is not prepared.
 * @param[in] period The Update period in seconds
 * @param[in] allow_duplicate @c 1 if it accepts the duplicated widget, otherwise @c 0
 * @param[in] result_cb The address of the callback function that is called when the result comes back from the viewer
 * @param[in] data The callback data that is used in the callback function
 *
 * @return #SHORTCUT_ERROR_NONE on success, other value on failure
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not established or there is a problem in the communication
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle a new request
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access the file or DB  Check your resource files
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a widget
 * @retval #SHORTCUT_ERROR_NOT_SUPPORTED Widget is not supported
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 * @see result_cb_t
 * @see shortcut_widget_size_e
 *
 * @par Example
 * @code
 *
 * #include <stdio.h>
 * #include <shortcut.h>
 *
 * static int result_cb(int ret, int pid, void *data)
 * {
 * 	if (ret < 0)
 * 		dlog_print("Failed to add a widget: %s\n", perror(ret));
 *
 *	dlog_print("Processed by the %d\n", pid);
 * 	return 0;
 * }
 *
 * static int app_create(void *data)
 * {
 * 	shortcut_add_to_home_widget("alter_name",
 * 					WIDGET_SIZE_1x1, "org.tizen.testwidget",
 * 					"/opt/media/Pictures/alter_icon.png", -1.0f, 0, result_cb, NULL);
 * 	return 0;
 * }
 *
 * @endcode
 */
extern int shortcut_add_to_home_widget(const char *name, shortcut_widget_size_e size, const char *widget_id, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data);


/**
 * @brief Called to receive the result of shortcut_get_list().
 * @since_tizen 2.4
 * @param[in] package_name The name of package
 * @param[in] icon The absolute path of an icon file for this shortcut
 * @param[in] name The name of the created shortcut icon
 * @param[in] extra_key The user data. A property of shortcut element in manifest file
 * @param[in] extra_data The user data. A property of shortcut element in manifest file
 * @param[in] user_data The callback user data
 * @return SHORTCUT_ERROR_NONE to continue with the next iteration of the loop, other error values to break out of the loop
 * @see shortcut_get_list()
 */
typedef int (*shortcut_list_cb)(const char *package_name, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *user_data);

/**
 * @brief Gets the installed shortcut view list, synchronously.
 * @remarks If a homescreen does not support this feature, you will get a proper error code.\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 *
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 * @param[in] package_name The package name
 * @param[in] shortcut_list_cb The callback function to get the shortcut item information
 * @param[in] data The callback data that is used in the callback function
 *
 * @return The return type (int)
 * @retval @c N Number of items (call count of the callback function)
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access the file or DB. Check your resource files
 * @pre You have to prepare the callback function.
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 */
extern int shortcut_get_list(const char *package_name, shortcut_list_cb list_cb, void *data);

/**
 * @brief Called to the add_to_home request.
 * @details The homescreen should define a callback as this type and implement the service code
 *        for adding a new application shortcut.
 * @since_tizen 2.4
 * @param[in] package_name The name of package
 * @param[in] name The name of the created shortcut icon
 * @param[in] type One of the three defined types
 * @param[in] content_info The specific information for creating a new shortcut
 * @param[in] icon The absolute path of an icon file for this shortcut
 * @param[in] pid The process ID of who request add_to_home
 * @param[in] allow_duplicate @c 1 if the shortcut can be duplicated,
 *                            otherwise a shourtcut should exist only once
 * @param[in] data The callback data
 * @return The result of handling a shortcut creation request\n
 *             This returns @c 0 if the add_to_home request is handled successfully,
 *             otherwise it returns a proper errno.
 * @see shortcut_set_request_cb()
 */
typedef int (*shortcut_request_cb)(const char *package_name, const char *name, int type, const char *content_info, const char *icon, int pid, double period, int allow_duplicate, void *data);

/**
 * @brief Registers a callback function to listen requests from applications.
 * @remarks Should be used in the homescreen.\n
 * Should check the return value of this function.
  * Prospective Clients: Homescreen.
  * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @param[in] request_cb The callback function pointer that is invoked when add_to_home is requested
 * @param[in] data The callback data to deliver to the callback function
 *
 * @return #SHORTCUT_ERROR_NONE on success, other value on failure
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not established or there is a problem in the communication
 * @pre You have to prepare a callback function.
 *
 * @post If a request is sent from the application, the registered callback will be invoked.
 *
 * @see request_cb_t
 * @see shortcut_error_e
 */
extern int shortcut_set_request_cb(shortcut_request_cb request_cb, void *data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
