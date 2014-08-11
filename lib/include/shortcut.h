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

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file shortcut.h
 * @brief This file declares API of libshortcut library
 */

/**
 * @addtogroup SHORTCUT_MODULE
 * @{
 */

/**
 * @brief Called to the add_to_home request.
 * @details The homescreen should define a callback as this type and implement the service code
 *        for adding a new application shortcut.
 * @since_tizen 2.3
 * @param[in] appid Shortcut is added for this package
 * @param[in] name Name for created shortcut icon
 * @param[in] type 3 kinds of types are defined
 * @param[in] content_info Specific information for creating a new shortcut
 * @param[in] icon Absolute path of an icon file for this shortcut
 * @param[in] pid Process ID of who request add_to_home
 * @param[in] allow_duplicate 1 if shortcut can be duplicated or a shourtcut should be exists only one
 * @param[in] data Callback data
 * @return Developer should returns the result of handling shortcut creation request\n
 *             Returns 0, if succeed to handles the add_to_home request, or returns proper errno
 * @see shortcut_set_request_cb()
 */
typedef int (*request_cb_t)(const char *appid, const char *name, int type, const char *content_info, const char *icon, int pid, double period, int allow_duplicate, void *data);

/**
 * @brief Called to receive the result of add_to_home_shortcut().
 * @since_tizen 2.3
 * @param[in] ret Result value, it could be 0 if succeed to add a shortcut, or errno
 * @param[in] pid Process ID of who handles this add_to_home request
 * @param[in] data Callback data
 * @return int Returns 0, if there is no error or returns errno
 * @see add_to_home_shortcut()
 */
typedef int (*result_cb_t)(int ret, int pid, void *data);

/**
 * @brief Enumeration for shortcut types.
 * @details Basically, three types of shortcut is defined.
 *        Every homescreen developer should support these types of shortcut.
 *        Or return proper errno to figure out why the application failed to add a shortcut.
 *        LAUNCH_BY_PACKAGE is used for adding a package itself as a shortcut.
 *        LAUNCH_BY_URI is used for adding a shortcut for "uri" data.
 * @since_tizen 2.3
 */
enum shortcut_type {
	/**< Deprecated type */
	SHORTCUT_PACKAGE	= 0x00000000,	/**< Launch the package using given pakcage name. */
	SHORTCUT_DATA		= 0x00000001,	/**< Launch the related package with given data(content_info). */
	SHORTCUT_FILE		= 0x00000002,	/**< Launch the related package with given filename(content_info). */

	/**< Use these */
	LAUNCH_BY_PACKAGE	= 0x00000000,	/**< Launch the package using given pakcage name. */
	LAUNCH_BY_URI		= 0x00000001,	/**< Launch the related package with given data(URI). */

	SHORTCUT_REMOVE		= 0x40000000,	/**< Remove a shortcut */
	DYNAMICBOX_REMOVE		= 0x80000000,	/**< Remove a dynamicbox */

	DYNAMICBOX_TYPE_DEFAULT	  = 0x10000000,	/**< Type mask for default dynamicbox */
	DYNAMICBOX_TYPE_EASY_DEFAULT = 0x30000000,	/**< Type mask for easy mode dynamicbox */
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
 * @brief Enumeration for values of type of shortcut response.
 * @since_tizen 2.3
 */
enum shortcut_error_e {
	SHORTCUT_ERROR_NONE = 0x00000000,				/**< Successfully handled */
	SHORTCUT_ERROR = 0x80000000,				/**< MSB(1). Check this using SHORTCUT_STATUS_IS_ERROR macro  */
	SHORTCUT_ERROR_NO_SPACE = SHORTCUT_ERROR | 0x0001,	/**< There is no space to add new shortcut */
	SHORTCUT_ERROR_EXIST = SHORTCUT_ERROR | 0x0002,		/**< Shortcut is already added */
	SHORTCUT_ERROR_FAULT = SHORTCUT_ERROR | 0x0004,		/**< Failed to add a shortcut. Unrecoverable error */
	SHORTCUT_ERROR_UNSUPPORTED = SHORTCUT_ERROR | 0x0008,	/**< Unsupported shortcut */
	SHORTCUT_ERROR_BUSY = SHORTCUT_ERROR | 0x0010,		/**< Receiver is busy, try again later */
	SHORTCUT_ERROR_INVALID_PARAMETER = SHORTCUT_ERROR | 0x0020,	/**< Shortcut request is not valid, invalid parameter or invalid argument value */
	SHORTCUT_ERROR_COMM = SHORTCUT_ERROR | 0x0040,		/**< Connection is not estabilished. or there is a problem of communication */ 
	SHORTCUT_ERROR_OUT_OF_MEMORY = SHORTCUT_ERROR | 0x0080,	/**< Memory is not enough to handle new request */
	SHORTCUT_ERROR_IO_ERROR = SHORTCUT_ERROR | 0x0100,		/**< Unable to access file or DB. Check your resource files */
	SHORTCUT_ERROR_PERMISSION_DENIED = SHORTCUT_ERROR | 0x0200,	/**< Has no permission to add a shortcut */

	SHORTCUT_STATUS_CARED = 0x08000000,			/**< Shortcut status is already cared. check this using SHORTCUT_STATUS_IS_CARED macro */
};

/**
 * @brief Definition for a macro to check the type.
 * @since_tizen 2.3
 * @param[in] type Type of box
 * @return bool
 * @retval true(1) If it is a dynamicbox
 * @retval false(0) if it is not a dynamicbox
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_DYNAMICBOX(type)	(!!((type) & 0x10000000))

/**
 * @brief Definition for a macro to check the request type.
 * @since_tizen 2.3
 * @param[in] type Request type
 * @return bool
 * @retval true(1) Shortcut remove request
 * @retval false(0) Not a remove request
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_REMOVE_SHORTCUT(type)	(!!((type) & SHORTCUT_REMOVE))

/**
 * @brief Definition for a macro to check the request type.
 * @since_tizen 2.3
 * @param[in] type Request type
 * @return bool
 * @retval true(1) Dynamicbox remove request
 * @retval false(0) Not a remove request
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_REMOVE_DYNAMICBOX(type)	(!!((type) & DYNAMICBOX_REMOVE))

/**
 * @brief Definition for a macro to check the request status.
 * @since_tizen 2.3
 * @param[in] type Status
 * @return bool
 * @retval true(1) Error
 * @retval false(0) Not an error
 * @see shortcut_error_e
 */
#define SHORTCUT_STATUS_IS_ERROR(type)	(!!((type) & SHORTCUT_ERROR))

/**
 * @brief Definition for a macro to check the request status.
 * @since_tizen 2.3
 * @param[in] type Status
 * @return bool
 * @retval true(1) Shortcut request is already handled by requestee (homescreen, viewer, ...)
 * @retval false(0) Request result should be cared by requestor
 * @see shortcut_error_e
 */
#define SHORTCUT_STATUS_IS_CARED(type)	(!!((type) & SHORTCUT_STATUS_CARED))

/**
 * @brief Definition for filtering the pure error code from given status.
 * @since_tizen 2.3
 * @param[in] status status
 * @return status code (error)
 * @see shortcut_error_e
 */
#define SHORTCUT_ERROR_CODE(status)	((status) & ~SHORTCUT_STATUS_CARED)

/**
 * @brief Supports the shortcut creating request.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * Important Notes: \n
 * Should be used from the homescreen.\n
 * Should check the return value of this function.
 *
 * Prospective Clients:
 * Homescreen.
 *
 * @since_tizen 2.3
 *
 * @privlevel public
 * @privilege http://tizen.org/privilege/shortcut
 *
 * @param[in] request_cb Callback function pointer which will be invoked when add_to_home is requested
 * @param[in] data Callback data to deliver to the callback function
 *
 * @return Return Type (int)
 * @retval 0 Callback function is successfully registered
 * @retval <0 Failed to register the callback function for request
 *
 * @pre You have to prepare a callback function.
 *
 * @post If a request is sent from the application, the registered callback will be invoked.
 *
 * @see request_cb_t
 * @par Example
 * @code
 * #include <shortcut.h>
 *
 * static int request_cb(const char *appid, const char *name, int type, const char *content_info, const char *icon, int pid, void *data)
 * {
 * 	printf("Package name: %s\n", appid);
 * 	printf("Name: %s\n", name);
 * 	printf("Type: %d\n", type);
 * 	printf("Content: %s\n", content_info);
 * 	printf("Icon: %s\n", icon);
 * 	printf("Requested from: %d\n", pid);
 * 	printf("CBDATA: %p\n", data);
 * 	return 0; // returns success.
 * }
 *
 * static int app_create(void *data)
 * {
 * 	shortcut_set_request_cb(request_cb, NULL);
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
extern int shortcut_set_request_cb(request_cb_t request_cb, void *data);

/**
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
 * @privilege http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get proper error code.
 * @param[in] appid Package name of owner of this shortcut
 * @param[in] name Name for created shortcut icon
 * @param[in] type Type of shortcuts (dynamicbox or shortcut, and its size if it is for the dynamicbox)
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon Absolute path of an icon file
 * @param[in] allow_duplicate set 1 If accept the duplicated shortcut or 0
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * @retval 0 Succeed to send the request
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle new request
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access file or DB. Check your resource files
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @see result_cb_t
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
 * 	add_to_home_shortcut("com.samsung.gallery", "With friends",
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
extern int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data);

/**
 *
 * @internal
 *
 * @brief Gets the installed shortcut view list.
 *
 * @details
 * Sync (or) Async:
 * This is a synchronous API.
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
 * @privilege http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get proper error code.
 * @param[in] appid Package name
 * @param[in] cb Callback function to get the shortcut item information
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * @retval @c N Number of items (call count of callback function)
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access file or DB. Check your resource files
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @see result_cb_t
 */
extern int shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data);

/**
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
 * @privilege http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get proper error code.
 * @param[in] appid Package name of owner of this shortcut
 * @param[in] name Name for created shortcut icon
 * @param[in] type Type of shortcuts (dynamicbox or shortcut, and its size if it is for the dynamicbox)
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut
 * @param[in] icon Absolute path of an icon file
 * @param[in] period Update period
 * @param[in] allow_duplicate Set 1 If accept the duplicated shortcut or 0
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * @retval 0 Succeed to send the request
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * @retval #SHORTCUT_ERROR_COMM Connection is not estabilished or there is a problem of communication
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle new request
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access file or DB  Check your resource files
 * @retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @see result_cb_t
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
 * 	add_to_home_dynamicbox("com.samsung.gallery.dynamicbox", "With friends",
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
extern int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data);

/**
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * Important Notes:\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function\n
 * Application should set the callback function to get the result of this request.
 *
 * Prospective Clients:
 * Inhouse Apps.
 *
 * @since_tizen 2.3
 *
 * @remarks - If a homescreen does not support this feature, you will get proper error code.
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut.
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval #SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle new request
 * \retval #SHORTCUT_ERROR_IO_ERROR Unable to access file or DB. Check your resource files
 * \retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @see result_cb_t
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
 * 	add_to_home_remove_shortcut("com.samsung.gallery.dynamicbox", "With friends",
 * 					"gallery:0000-0000",
 * 					result_cb, NULL);
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
extern int add_to_home_remove_shortcut(const char *appid, const char *name, const char *content_info, result_cb_t result_cb, void *data);

/**
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @details
 * Sync (or) Async:
 * This is an asynchronous API.
 *
 * Important Notes:\n
 * Application must check the return value of this function.\n
 * Application must check the return status from the callback function\n
 * Application should set the callback function to get the result of this request.
 *
 * Prospective Clients:
 * Inhouse Apps.
 *
 * @since_tizen 2.3
 *
 * @remarks - If a homescreen does not support this feature, you will get proper error code.
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval #SHORTCUT_ERROR_INVALID_PARAMETER Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval #SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval #SHORTCUT_ERROR_OUT_OF_MEMORY Memory is not enough to handle new request
 * \retval #SHORTCUT_ERROR_IO_ERROR Unable to access file or DB. Check your resource files
 * \retval #SHORTCUT_ERROR_PERMISSION_DENIED Has no permission to add a shortcut
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @see result_cb_t
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
 * 	add_to_home_remove_dynamicbox("com.samsung.gallery.dynamicbox", "With friends", result_cb, NULL);
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
extern int add_to_home_remove_dynamicbox(const char *appid, const char *name, result_cb_t result_cb, void *data);

#ifdef __cplusplus
}
#endif

#endif
/* @}
 * End of a file 
 */
