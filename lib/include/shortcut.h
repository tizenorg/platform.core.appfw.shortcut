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

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

#include <tizen.h>
#include <shortcut_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file shortcut.h
 * @brief This file declares the API of the libshortcut library.
 */

/**
 * @addtogroup SHORTCUT_MODULE
 * @{
 */

/**
 * @brief Called to receive the result of add_to_home_shortcut().
 * @since_tizen 2.3
 * @param[in] ret The result value, it could be @c 0 if it succeeds to add a shortcut,
 *                otherwise it returns an errno
 * @param[in] pid The process ID of who handle this add_to_home request
 * @param[in] data The callback data
 * @return int @c 0 if there is no error,
               otherwise errno
 * @see add_to_home_shortcut()
 */
typedef int (*result_internal_cb_t)(int ret, int pid, void *data);

/**
 * @brief Enumeration for shortcut types.
 * @details Basically, three types of shortcuts are defined.
 *          Every homescreen developer should support these types of shortcuts.
 *          Or return a proper errno to figure out why the application failed to add a shortcut.
 *          #LAUNCH_BY_PACKAGE is used for adding a package itself as a shortcut.
 *          #LAUNCH_BY_URI is used for adding a shortcut for "uri" data.
 * @since_tizen 2.3
 */
enum shortcut_internal_type {
	/**< Deprecated type */
	SHORTCUT_PACKAGE	= 0x00000000,	/**< Launch the package using the given package name */
	SHORTCUT_DATA		= 0x00000001,	/**< Launch the related package with the given data(content_info) */
	SHORTCUT_FILE		= 0x00000002,	/**< Launch the related package with the given filename(content_info) */

	LAUNCH_BY_PACKAGE	= 0x00000000,

	SHORTCUT_REMOVE		= 0x40000000,	    /**< Remove a shortcut */
	DYNAMICBOX_REMOVE		= 0x80000000,	/**< Remove a widget */

	DYNAMICBOX_TYPE_DEFAULT	  = 0x10000000,	/**< Type mask for the default widget */
	DYNAMICBOX_TYPE_EASY_DEFAULT = 0x30000000,	/**< Type mask for the easy mode widget */
	DYNAMICBOX_TYPE_1x1	  = 0x10010000,	/**< 1x1 */
	DYNAMICBOX_TYPE_2x1	  = 0x10020000,	/**< 2x1 */
	DYNAMICBOX_TYPE_2x2	  = 0x10040000,	/**< 2x2 */
	DYNAMICBOX_TYPE_4x1	  = 0x10080000,	/**< 4x1 */
	DYNAMICBOX_TYPE_4x2	  = 0x10100000,	/**< 4x2 */
	DYNAMICBOX_TYPE_4x3  	  = 0x10200000,	/**< 4x3 */
	DYNAMICBOX_TYPE_4x4	  = 0x10400000,	/**< 4x4 */
	DYNAMICBOX_TYPE_4x5	  = 0x11000000,	/**< 4x5 */
	DYNAMICBOX_TYPE_4x6	  = 0x12000000, /**< 4x6 */
	DYNAMICBOX_TYPE_EASY_1x1	  = 0x30010000,	/**< Easy mode 1x1 */
	DYNAMICBOX_TYPE_EASY_3x1	  = 0x30020000,	/**< Easy mode 3x2 */
	DYNAMICBOX_TYPE_EASY_3x3	  = 0x30040000,	/**< Easy mode 3x3 */
	DYNAMICBOX_TYPE_UNKNOWN	  = 0x1FFF0000,	/**< Error */
};

/**
 * @brief Enumeration for values of shortcut response types.
 * @since_tizen 2.3
 */
enum shortcut_internal_error_e {
	SHORTCUT_ERROR = 0x80000000,				/**< MSB(1). Check this using the #SHORTCUT_STATUS_IS_ERROR macro  */
	SHORTCUT_STATUS_CARED = 0x08000000,			/**< Shortcut status is already cared. Check this using the #SHORTCUT_STATUS_IS_CARED macro */
	SHORTCUT_ERROR_BUSY = TIZEN_ERROR_RESOURCE_BUSY,		/**< Receiver is busy, try again later */
	SHORTCUT_ERROR_UNSUPPORTED = SHORTCUT_ERROR | 0x0400	/**< Shortcut is not supported */
};

/**
 * @brief Definition for a macro to check type.
 * @since_tizen 2.3
 * @param[in] type The type of box
 * @return bool
 * @retval true(1) If it is a dynamicbox
 * @retval false(0) If it is not a dynamicbox
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_DYNAMICBOX(type)	(!!((type) & 0x10000000))


/**
 * @brief Definition for a macro to check the request status.
 * @since_tizen 2.3
 * @param[in] type The status
 * @return bool
 * @retval true(1) Error
 * @retval false(0) Not an error
 * @see shortcut_error_e
 */
#define SHORTCUT_STATUS_IS_ERROR(type)	(!!((type) & SHORTCUT_ERROR))

/**
 * @brief Definition for a macro to check the request status.
 * @since_tizen 2.3
 * @param[in] type The status
 * @return bool
 * @retval true(1) Shortcut request is already handled by the requestee (homescreen, viewer, ...)
 * @retval false(0) Request result should be cared by the requestor
 * @see shortcut_error_e
 */
#define SHORTCUT_STATUS_IS_CARED(type)	(!!((type) & SHORTCUT_STATUS_CARED))

/**
 * @brief Definition for filtering the pure error code from the given status.
 * @since_tizen 2.3
 * @param[in] status The status
 * @return The status code (error)
 * @see shortcut_error_e
 */
#define SHORTCUT_ERROR_CODE(status)	((status) & ~SHORTCUT_STATUS_CARED)

/**
 * @internal
 *
 * @brief Supports the add_to_home feature, should invoke this.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * Important Notes:\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 *
 * Prospective Clients:
 * Inhouse Apps.
 *
 * @since_tizen 2.3
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get a proper error code.
 * @param[in] appid The package name of the owner of this shortcut
 * @param[in] name The name of the created shortcut icon
 * @param[in] type The type of shortcuts (dynamicbox or shortcut, and its size if it is for the dynamicbox)
 * @param[in] content_info The specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon The absolute path of an icon file
 * @param[in] allow_duplicate @c 1 if it accepts the duplicated shortcut,
 *                            otherwise @c 0
 * @param[in] result_cb The address of the callback function that is called when the result comes back from the viewer
 * @param[in] data The callback data that is used in the callback function
 *
 * @return The return type (int)
 * @retval 0 Succeeded to send the request
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem in the communication
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle a new request
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access the file or DB. Check your resource files
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 * @retval #SHORTCUT_ERROR_NOT_SUPPORTED Shortcut is not supported
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 * @see result_internal_cb_t
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
 * 		printf("Failed to add a shortcut: %s\n", perror(ret));
 *
 *	printf("Processed by the %d\n", pid);
 * 	return 0;
 * }
 *
 * static int app_create(void *data)
 * {
 * 	add_to_home_shortcut("org.tizen.gallery", "With friends",
 * 					LAUNCH_BY_URI, "gallery:0000-0000",
 * 					"/opt/media/Pictures/Friends.jpg", 0, result_cb, NULL);
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
extern int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, int allow_duplicate, result_internal_cb_t result_cb, void *data);

/**
 * @internal
 *
 * @brief Supports the add_to_home feature, should invoke this.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * Important Notes:\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function.\n
 * Application should set the callback function to get the result of this request.
 *
 * Prospective Clients:
 * Inhouse Apps.
 *
 * @since_tizen 2.3
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get a proper error code.
 * @param[in] appid The package name of the owner of this shortcut
 * @param[in] name The name of the created shortcut icon
 * @param[in] type The type of shortcuts (dynamicbox or shortcut, and its size if it is for the dynamicbox)
 * @param[in] content_info The specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon The absolute path of an icon file
 * @param[in] period The update period
 * @param[in] allow_duplicate @c 1 if it accepts the duplicated shortcut,
 *                            otherwise @c 0
 * @param[in] result_cb The address of the callback function that is called when the result comes back from the viewer
 * @param[in] data The callback data that is used in the callback function
 *
 * @return The return type (int)
 * @retval 0 Succeeded to send the request
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not established or there is a problem in the communication
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle a new request
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access the file or DB  Check your resource files
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 * @retval #SHORTCUT_ERROR_NOT_SUPPORTED Shortcut is not supported
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 * @see result_internal_cb_t
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
 * 		printf("Failed to add a shortcut: %s\n", perror(ret));
 *
 *	printf("Processed by the %d\n", pid);
 * 	return 0;
 * }
 *
 * static int app_create(void *data)
 * {
 * 	add_to_home_dynamicbox("org.tizen.gallery.dynamicbox", "With friends",
 * 					LAUNCH_BY_URI, "gallery:0000-0000",
 * 					"/opt/media/Pictures/Friends.jpg", -1.0f, 0, result_cb, NULL);
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
extern int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
