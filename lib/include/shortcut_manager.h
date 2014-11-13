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
 *
 * @brief Supports the add_to_home feature, should invoke this.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * @since_tizen 2.3
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @remarks Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 * @remarks If a homescreen does not support this feature, you will get a proper error code.
 *
 * @param[in] name The name of the created shortcut icon
 * @param[in] type The type of shortcuts
 * @param[in] uri The specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon The absolute path of an icon file
 * @param[in] allow_duplicate @c 1 if it accepts the duplicated shortcut,
 *                            otherwise @c 0
 * @param[in] result_cb The address of the callback function that is called when the result comes back from the viewer
 * @param[in] data The callback data that is used in the callback function
 *
 * @return @c 0 on success, otherwise a negative error value
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
 * 		printf("Failed to add a shortcut: %s\n", perror(ret));
 *
 *	printf("Processed by the %d\n", pid);
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
 * int main(int argc, char *argv[])
 * {
 * 	appcore....
 * }
 *
 * @endcode
 */
extern int shortcut_add_to_home(const char *name, shortcut_type type, const char *uri, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
