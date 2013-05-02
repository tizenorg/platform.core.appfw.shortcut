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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup APPLICATION_FRAMEWORK
 * @{
 */

/**
 * @defgroup SHORTCUT Add to home (shortcut)
 * @author Sung-jae Park <nicesj.park@samsung.com>
 * @version 0.1
 * @brief To enhance the Add to home feature. Two types of API set are supported.
 *        One for the homescreen developers.
 *        The others for the application developers who should implement the Add to home feature.
 */

/**
 * @brief This function prototype is used to define a callback function for the add_to_home reqeust.
 *        The homescreen should define a callback as this type and implementing the service code
 *        for adding a new application shortcut.
 * @param[in] appid Shortcut is added for this package.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type 3 kinds of types are defined.
 * @param[in] content_info Specific information for creating a new shortcut.
 * @param[in] icon Absolute path of an icon file for this shortcut.
 * @param[in] pid Process ID of who request add_to_home.
 * @param[in] allow_duplicate 1 if shortcut can be duplicated or a shourtcut should be exists only one.
 * @param[in] data Callback data.
 * @return int Developer should returns the result of handling shortcut creation request.
 *             Returns 0, if succeed to handles the add_to_home request, or returns proper errno.
 * @see shortcut_set_request_cb
 * @pre None
 * @post None
 * @remarks None
 */
typedef int (*request_cb_t)(const char *appid, const char *name, int type, const char *content_info, const char *icon, int pid, double period, int allow_duplicate, void *data);

/**
 * @brief This function prototype is used to define for receiving the result of add_to_home.
 * @param[in] ret Result value, it could be 0 if succeed to add a shortcut, or errno.
 * @param[in] pid Process ID of who handles this add_to_home request.
 * @param[in] data Callback data.
 * @return int Returns 0, if there is no error or returns errno.
 * @see add_to_home_shortcut()
 * @pre None
 * @post None
 * @remarks None
 */
typedef int (*result_cb_t)(int ret, int pid, void *data);

/**
 * @brief Basically, three types of shortcut is defined.
 *        Every homescreen developer should support these types of shortcut.
 *        Or returns proper errno to figure out why the application failed to add a shortcut.
 *        LAUNCH_BY_PACKAGE is used for adding a package itself as a shortcut
 *        LAUNCH_BY_URI is used for adding a shortcut for "uri" data.
 */
enum shortcut_type {
	/*!< Deprecated type */
	SHORTCUT_PACKAGE	= 0x00000000, /**< Launch the package using given pakcage name. */
	SHORTCUT_DATA		= 0x00000001, /**< Launch the related package with given data(content_info). */
	SHORTCUT_FILE		= 0x00000002, /**< Launch the related package with given filename(content_info). */

	/*!< Use these */
	LAUNCH_BY_PACKAGE	= 0x00000000, /*!< Launch the package using given pakcage name. */
	LAUNCH_BY_URI		= 0x00000001, /*!< Launch the related package with given data(URI). */

	SHORTCUT_REMOVE		= 0x40000000, /*!< Remove a shortcut */
	LIVEBOX_REMOVE		= 0x80000000, /*!< Remove a livebox */

	LIVEBOX_TYPE_DEFAULT	  = 0x10000000,
	LIVEBOX_TYPE_EASY_DEFAULT = 0x30000000,
	LIVEBOX_TYPE_1x1	  = 0x10010000,
	LIVEBOX_TYPE_2x1	  = 0x10020000,
	LIVEBOX_TYPE_2x2	  = 0x10040000,
	LIVEBOX_TYPE_4x1	  = 0x10080000,
	LIVEBOX_TYPE_4x2	  = 0x10100000,
	LIVEBOX_TYPE_4x3  	  = 0x10200000,
	LIVEBOX_TYPE_4x4	  = 0x10400000,
	LIVEBOX_TYPE_4x5	  = 0x11000000,
	LIVEBOX_TYPE_4x6	  = 0x12000000,
	LIVEBOX_TYPE_EASY_1x1	  = 0x30010000,
	LIVEBOX_TYPE_EASY_3x1	  = 0x30020000,
	LIVEBOX_TYPE_EASY_3x3	  = 0x30040000,
	LIVEBOX_TYPE_UNKNOWN	  = 0x1FFF0000,
};

enum shortcut_response {
	SHORTCUT_SUCCESS = 0x00000000, /*!< Successfully handled */
	SHORTCUT_ERROR = 0x80000000,	/*!< MSB(1). Check this using SHORTCUT_STATUS_IS_ERROR macro  */
	SHORTCUT_ERROR_NO_SPACE = SHORTCUT_ERROR | 0x0001, /*!< There is no space to add new shortcut */
	SHORTCUT_ERROR_EXIST = SHORTCUT_ERROR | 0x0002, /*!< Shortcut is already added */
	SHORTCUT_ERROR_FAULT = SHORTCUT_ERROR | 0x0004, /*!< Failed to add a shortcut. Unrecoverable error */
	SHORTCUT_ERROR_UNSUPPORTED = SHORTCUT_ERROR | 0x0008, /*!< Unsupported shortcut */
	SHORTCUT_ERROR_BUSY = SHORTCUT_ERROR | 0x0010, /*!< Receiver is busy, try again later */
	SHORTCUT_ERROR_INVALID = SHORTCUT_ERROR | 0x0020, /*!< Shortcut request is not valid, invalid parameter or invalid argument value */
	SHORTCUT_ERROR_COMM = SHORTCUT_ERROR | 0x0040, /*!< Connection is not estabilished. or there is a problem of communication */ 
	SHORTCUT_ERROR_MEMORY = SHORTCUT_ERROR | 0x0080, /*!< Memory is not enough to handle new request */
	SHORTCUT_ERROR_IO = SHORTCUT_ERROR | 0x0100, /*!< Unable to access file or DB. Check your resource files */

	SHORTCUT_STATUS_CARED = 0x08000000, /*!< Shortcut status is already cared. check this using SHORTCUT_STATUS_IS_CARED macro */
};

#define ADD_TO_HOME_IS_LIVEBOX(type)	(!!((type) & 0x10000000))
#define ADD_TO_HOME_IS_REMOVE_REQUEST(type)	(!!((type) & SHORTCUT_REMOVE))

#define SHORTCUT_STATUS_IS_ERROR(type)	(!!((type) & SHORTCUT_ERROR))
#define SHORTCUT_STATUS_IS_CARED(type)	(!!((type) & SHORTCUT_STATUS_CARED))
#define SHORTCUT_ERROR_CODE(type)	(type & ~SHORTCUT_STATUS_CARED)

/**
 * @fn int shortcut_set_request_cb(request_cb_t request_cb, void *data)
 *
 * @brief Homescreen should use this function to service the shortcut creating request.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Should be used from the homescreen.
 * - Should check the return value of this function
 *
 * @param[in] request_cb Callback function pointer which will be invoked when add_to_home is requested.
 * @param[in] data Callback data to deliver to the callback function.
 *
 * @return Return Type (int)
 * - 0 - callback function is successfully registered
 * - < 0 - Failed to register the callback function for request.
 *
 * @see request_cb_t
 *
 * @pre - You have to prepare a callback function
 *
 * @post - If a request is sent from the application, the registered callback will be invoked.
 *
 * @remarks - None
 *
 * @par Prospective Clients:
 * Homescreen
 *
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
 * @fn int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, result_cb_t result_cb, void *data)
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Application should check the return value of this function.
 * - Application should check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type 3 kinds of types are defined.
 * @param[in] content_info Specific information for delivering to the creating shortcut.
 * @param[in] icon Absolute path of an icon file
 * @param[in] allow_duplicate set 1 If accept the duplicated shortcut or 0
 * @param[in] result_cb Callback function pointer which will be invoked after add_to_home request.
 * @param[in] data Callback data to deliver to the callback function.
 *
 * @return Return Type (int)
 * - 0 - Succeed to send the request
 * - <0 - Failed to send the request
 *
 * @see result_cb_t
 *
 * @pre - You have to prepare the callback function
 *
 * @post - You have to check the return status from callback function which is passed by argument.
 *
 * @remarks - If a homescreen does not support this feature, you will get proper error code.
 *
 * @par Prospective Clients:
 * Inhouse Apps.
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
 * 					SHORTCUT_DATA, "gallery:0000-0000",
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
extern int shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data);

extern int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data);

extern int add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data);

extern int add_to_home_remove_shortcut(const char *appid, const char *name, const char *content_info, result_cb_t result_cb, void *data);

extern int add_to_home_remove_livebox(const char *appid, const char *name, result_cb_t result_cb, void *data);

#ifdef __cplusplus
}
#endif

#endif
/* @}
 * End of a file 
 */
