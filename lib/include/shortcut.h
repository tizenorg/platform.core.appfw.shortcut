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
 * @addtogroup SHORTCUT_MODULE
 * @{
 */

struct shortcut_icon;
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
 * @brief After send a request to create a icon snapshot image, this callback will be called with its result.
 * @param[in] handle Handle of requestor
 * @param[in] ret status of request
 * @param[in] data Callback data
 * @return int result state of callback call
 * @retval 0 If it is successfully completed
 * @see shortcut_icon_request_send()
 * @pre None
 * @post None
 * @remarks None
 */
typedef int (*icon_request_cb_t)(struct shortcut_icon *handle, int ret, void *data);

/**
 * @brief Basically, three types of shortcut is defined.
 *        Every homescreen developer should support these types of shortcut.
 *        Or returns proper errno to figure out why the application failed to add a shortcut.
 *        LAUNCH_BY_PACKAGE is used for adding a package itself as a shortcut
 *        LAUNCH_BY_URI is used for adding a shortcut for "uri" data.
 */
//! [Enumeration values for type of shortcuts]
enum shortcut_type {
	/*!< Deprecated type */
	SHORTCUT_PACKAGE	= 0x00000000,	/*!< Launch the package using given pakcage name. */
	SHORTCUT_DATA		= 0x00000001,	/*!< Launch the related package with given data(content_info). */
	SHORTCUT_FILE		= 0x00000002,	/*!< Launch the related package with given filename(content_info). */

	/*!< Use these */
	LAUNCH_BY_PACKAGE	= 0x00000000,	/*!< Launch the package using given pakcage name. */
	LAUNCH_BY_URI		= 0x00000001,	/*!< Launch the related package with given data(URI). */

	SHORTCUT_REMOVE		= 0x40000000,	/*!< Remove a shortcut */
	LIVEBOX_REMOVE		= 0x80000000,	/*!< Remove a livebox */

	LIVEBOX_TYPE_DEFAULT	  = 0x10000000,	/*!< Type mask for default livebox */
	LIVEBOX_TYPE_EASY_DEFAULT = 0x30000000,	/*!< Type mask for easy mode livebox */
	LIVEBOX_TYPE_1x1	  = 0x10010000,	/*!< 1x1 */
	LIVEBOX_TYPE_2x1	  = 0x10020000,	/*!< 2x1 */
	LIVEBOX_TYPE_2x2	  = 0x10040000,	/*!< 2x2 */
	LIVEBOX_TYPE_4x1	  = 0x10080000,	/*!< 4x1 */
	LIVEBOX_TYPE_4x2	  = 0x10100000,	/*!< 4x2 */
	LIVEBOX_TYPE_4x3  	  = 0x10200000,	/*!< 4x3 */
	LIVEBOX_TYPE_4x4	  = 0x10400000,	/*!< 4x4 */
	LIVEBOX_TYPE_4x5	  = 0x11000000,	/*!< 4x5 */
	LIVEBOX_TYPE_4x6	  = 0x12000000, /*!< 4x6 */
	LIVEBOX_TYPE_EASY_1x1	  = 0x30010000,	/*!< Easy mode 1x1 */
	LIVEBOX_TYPE_EASY_3x1	  = 0x30020000,	/*!< Easy mode 3x2 */
	LIVEBOX_TYPE_EASY_3x3	  = 0x30040000,	/*!< Easy mode 3x3 */
	LIVEBOX_TYPE_UNKNOWN	  = 0x1FFF0000	/*!< Error */
};
//! [Enumeration values for type of shortcuts]

enum shortcut_response {
	SHORTCUT_SUCCESS = 0x00000000,				/*!< Successfully handled */
	SHORTCUT_ERROR = 0x80000000,				/*!< MSB(1). Check this using SHORTCUT_STATUS_IS_ERROR macro  */
	SHORTCUT_ERROR_NO_SPACE = SHORTCUT_ERROR | 0x0001,	/*!< There is no space to add new shortcut */
	SHORTCUT_ERROR_EXIST = SHORTCUT_ERROR | 0x0002,		/*!< Shortcut is already added */
	SHORTCUT_ERROR_FAULT = SHORTCUT_ERROR | 0x0004,		/*!< Failed to add a shortcut. Unrecoverable error */
	SHORTCUT_ERROR_UNSUPPORTED = SHORTCUT_ERROR | 0x0008,	/*!< Unsupported shortcut */
	SHORTCUT_ERROR_BUSY = SHORTCUT_ERROR | 0x0010,		/*!< Receiver is busy, try again later */
	SHORTCUT_ERROR_INVALID = SHORTCUT_ERROR | 0x0020,	/*!< Shortcut request is not valid, invalid parameter or invalid argument value */
	SHORTCUT_ERROR_COMM = SHORTCUT_ERROR | 0x0040,		/*!< Connection is not estabilished. or there is a problem of communication */ 
	SHORTCUT_ERROR_MEMORY = SHORTCUT_ERROR | 0x0080,	/*!< Memory is not enough to handle new request */
	SHORTCUT_ERROR_IO = SHORTCUT_ERROR | 0x0100,		/*!< Unable to access file or DB. Check your resource files */
	SHORTCUT_ERROR_PERMISSION = SHORTCUT_ERROR | 0x0200,	/*!< Has no permission to add a shortcut */

	SHORTCUT_STATUS_CARED = 0x08000000			/*!< Shortcut status is already cared. check this using SHORTCUT_STATUS_IS_CARED macro */
};

/*!
 * \brief Macro function for checking the type
 * \param[in] type Type of box
 * \return bool
 * \retval true(1) If it is a livebox
 * \retval false(0) if it is not a livebox
 * \see shortcut_type
 * \pre None
 * \post None
 * \remarks None
 */
#define ADD_TO_HOME_IS_LIVEBOX(type)	(!!((type) & 0x10000000))

/*!
 * \brief Macro function for checking the request type
 * \param[in] type Request type
 * \return bool
 * \retval true(1) Shortcut remove request
 * \retval false(0) Not a remove request
 * \see shortcut_type
 * \pre None
 * \post None
 * \remarks None
 */
#define ADD_TO_HOME_IS_REMOVE_SHORTCUT(type)	(!!((type) & SHORTCUT_REMOVE))

/*!
 * \brief Macro function for checking the request type
 * \param[in] type Request type
 * \return bool
 * \retval true(1) Livebox remove request
 * \retval false(0) Not a remove request
 * \see shortcut_type
 * \pre None
 * \post None
 * \remarks None
 */
#define ADD_TO_HOME_IS_REMOVE_LIVEBOX(type)	(!!((type) & LIVEBOX_REMOVE))

/*!
 * \brief Macro function for checking the status of request
 * \param[in] type Status
 * \return bool
 * \retval true(1) Error
 * \retval false(0) Not an error
 * \see shortcut_response
 * \pre None
 * \post None
 * \remarks None
 */
#define SHORTCUT_STATUS_IS_ERROR(type)	(!!((type) & SHORTCUT_ERROR))

/*!
 * \brief Macro function for checking the status of request
 * \param[in] type Status
 * \return bool
 * \retval true(1) Shortcut request is already handled by requestee (homescreen, viewer, ...)
 * \retval false(0) Request result should be cared by requestor
 * \see shortcut_response
 * \pre None
 * \post None
 * \remarks None
 */
#define SHORTCUT_STATUS_IS_CARED(type)	(!!((type) & SHORTCUT_STATUS_CARED))

/*!
 * \brief Filtering the pure error code from given status
 * \param[in] status status
 * \return status code (error)
 * \see shortcut_response
 * \pre None
 * \post None
 * \remarks None
 *
 */
#define SHORTCUT_ERROR_CODE(status)	((status) & ~SHORTCUT_STATUS_CARED)

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
 * @fn add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, int allow_duplicate, result_cb_t result_cb, void *data)
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Application must check the return value of this function.
 * - Application must check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type Type of shortcuts (livebox or shortcut, and its size if it is for the livebox)
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut.
 * @param[in] icon Absolute path of an icon file
 * @param[in] allow_duplicate set 1 If accept the duplicated shortcut or 0
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval SHORTCUT_ERROR_INVALID Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval SHORTCUT_ERROR_MEMORY Memory is not enough to handle new request
 * \retval SHORTCUT_ERROR_IO Unable to access file or DB. Check your resource files
 * \retval SHORTCUT_ERROR_PERMISSION Has no permission to add a shortcut
 *
 * @see result_cb_t
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
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
 * @fn shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data)
 *
 * @brief Getting the installed shortcut view list
 *
 * @par Sync (or) Async:
 * This is a synchronous API.
 *
 * @par Important Notes:
 * - Application must check the return value of this function.
 * - Application must check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name
 * @param[in] cb Callback function to get the shortcut item information
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval Number of items (call count of callback function) 
 * \retval SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval SHORTCUT_ERROR_IO Unable to access file or DB. Check your resource files
 *
 * @see result_cb_t
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
 *
 * @remarks - If a homescreen does not support this feature, you will get proper error code.
 *
 * @par Prospective Clients:
 * Inhouse Apps.
 *
 * @par Example
 * @code
 * @endcode
 */
extern int shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data);

/**
 * @fn add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data);
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Application must check the return value of this function.
 * - Application must check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type Type of shortcuts (livebox or shortcut, and its size if it is for the livebox)
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut.
 * @param[in] icon Absolute path of an icon file
 * @param[in] period Update period
 * @param[in] allow_duplicate set 1 If accept the duplicated shortcut or 0
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval SHORTCUT_ERROR_INVALID Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval SHORTCUT_ERROR_MEMORY Memory is not enough to handle new request
 * \retval SHORTCUT_ERROR_IO Unable to access file or DB. Check your resource files
 * \retval SHORTCUT_ERROR_PERMISSION Has no permission to add a shortcut
 *
 * @see result_cb_t
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
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
 * 	add_to_home_livebox("com.samsung.gallery.livebox", "With friends",
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
extern int add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_cb_t result_cb, void *data);

/*!
 * @fn add_to_home_remove_shortcut(const char *appid, const char *name, const char *content_info, result_cb_t result_cb, void *data);
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Application must check the return value of this function.
 * - Application must check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] content_info Specific information for delivering to the viewer for creating a shortcut.
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval SHORTCUT_ERROR_INVALID Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval SHORTCUT_ERROR_MEMORY Memory is not enough to handle new request
 * \retval SHORTCUT_ERROR_IO Unable to access file or DB. Check your resource files
 * \retval SHORTCUT_ERROR_PERMISSION Has no permission to add a shortcut
 *
 * @see result_cb_t
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
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
 * 	add_to_home_remove_shortcut("com.samsung.gallery.livebox", "With friends",
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

/*!
 * @fn add_to_home_remove_livebox(const char *appid, const char *name, result_cb_t result_cb, void *data);
 *
 * @brief The application, which supporting the add_to_home feature, should invoke this.
 *
 * @par Sync (or) Async:
 * This is an asynchronous API.
 *
 * @par Important Notes:
 * - Application must check the return value of this function.
 * - Application must check the return status from the callback function
 * - Application should set the callback function to get the result of this request.
 *
 * @param[in] appid Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] result_cb Address of callback function which will be called when the result comes back from the viewer.
 * @param[in] data Callback data which will be used in callback function
 *
 * @return Return Type (int)
 * \retval 0 Succeed to send the request
 * \retval SHORTCUT_ERROR_FAULT Unrecoverable error
 * \retval SHORTCUT_ERROR_INVALID Shortcut request is not valid, invalid parameter or invalid argument value
 * \retval SHORTCUT_ERROR_COMM Connection is not estabilished. or there is a problem of communication
 * \retval SHORTCUT_ERROR_MEMORY Memory is not enough to handle new request
 * \retval SHORTCUT_ERROR_IO Unable to access file or DB. Check your resource files
 * \retval SHORTCUT_ERROR_PERMISSION Has no permission to add a shortcut
 *
 * @see result_cb_t
 *
 * @pre You have to prepare the callback function
 *
 * @post You have to check the return status from callback function which is passed by argument.
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
 * 	add_to_home_remove_livebox("com.samsung.gallery.livebox", "With friends", result_cb, NULL);
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
extern int add_to_home_remove_livebox(const char *appid, const char *name, result_cb_t result_cb, void *data);



/*!
 * \note
 * Example)
 *
 * \code
 * static int init_cb(int status, void *data)
 * {
 *    printf("Initializer returns: %d\n", status);
 *    if (status == 0) {
 *        printf("Succeed to initialize\n");
 *    } else {
 *        printf("Failed to initialize: %d\n", status);
 *    }
 * }
 *
 * int main(int argc, char *argv[])
 * {
 *     // Initialize the service request
 *     int ret;
 *
 *     // After the init_cb is called, you can use below functions.
 *     struct shortcut_icon *handle;
 *
 *     ret = shortcut_icon_init(init_cb, NULL);
 *     if (ret < 0) {
 *        ...
 *
 *     // Create request for creating shortcut icon.
 *     handle = shortcut_icon_create();
 *     if (!handle) {
 *         ...
 *     }
 * 
 *     // Send the request to the shortcut service
 *     ret = shortcut_icon_request_set_info(handle, NULL, SHORTCUT_ICON_TYPE_IMAGE, "icon, "/usr/share/.../icon.png", NULL, NULL);
 *     if (ret < 0) {
 *        ...
 *     }
 *
 *     ret = shortcut_icon_request_set_info(handle, NULL, SHORTCUT_ICON_TYPE_TEXT, "text, "app icon", NULL, NULL);
 *     if (ret < 0) {
 *        ...
 *     }
 *
 *     ret = shortcut_icon_request_send(handle, LB_SIZE_TYPE_1x1, NULL, NULL, "/opt/usr/apps/com.samsung.cluster-home/data/out.png", result_cb, NULL);
 *     if (ret < 0) {
 *        ...
 *     }
 *
 *     ret = shortcut_icon_request_destroy(handle);
 *     if (ret < 0) {
 *        ...
 *     }
 *
 *     // Don't finalize the icon service if you don't get result callbacks of all requests
 *     ret = shortcut_icon_fini();
 *     if (ret < 0) {
 *        ...
 *     }
 *
 *     return 0;
 * }
 * \endcode
 */

#define DEFAULT_ICON_PART		"icon"
#define DEFAULT_NAME_PART		"name"
#define SHORTCUT_ICON_TYPE_IMAGE	"image"
#define SHORTCUT_ICON_TYPE_TEXT		"text"
#define SHORTCUT_ICON_TYPE_SCRIPT	"script"

/*!
 * \brief Initialize the icon creation service
 * \remarks N/A
 * \details N/A
 * \param[in] init_cb Initialized result will be delievered via this callback
 * \param[in] data Callback data
 * \pre N/A
 * \post N/A
 * \return int
 * \retval SHORTCUT_ERROR_INVALID Already initialized
 * \retval SHORTCUT_ERROR_SUCCESS Successfully initialized
 * \see shortcut_icon_service_fini
 */
extern int shortcut_icon_service_init(int (*init_cb)(int status, void *data), void *data);

/*!
 * \brief Finalize the icon creation service
 * \remarks N/A
 * \details N/A
 * \pre N/A
 * \post N/A
 * \return int
 * \retval SHORTCUT_SUCCESS Successfully initialized
 * \retval SHORTCUT_ERROR_INVALID icon service is not initialized
 * \see shortcut_icon_service_init
 */
extern int shortcut_icon_service_fini(void);

/*!
 * \brief Create a request object to create a new icon image
 * \remarks N/A
 * \details N/A
 * \pre N/A
 * \post N/A
 * \return struct shortcut_icon *
 * \retval NULL If it fails to create a new handle
 * \retval pointer Handle address
 * \see shortcut_icon_request_destroy
 */
extern struct shortcut_icon *shortcut_icon_request_create(void);

/*!
 * \brief Set infomration for creating icon image
 * \details N/A
 * \remarks N/A
 * \param[in] handle Request handle
 * \param[in] id Target ID to be affected by this data
 * \param[in] type SHORTCUT_ICON_TYPE_IMAGE, SHORTCUT_ICON_TYPE_TEXT, SHORTCUT_ICON_TYPE_SCRIPT can be used
 * \param[in] part Target part to be affect by this data
 * \param[in] data type == IMAGE ? Image file path : type == TEXT ? text string : type == SCRIPT ? script file path : N/A
 * \param[in] option Image load option or group name of script file to be loaded
 * \param[in] subid ID for script. this ID will be used as "id"
 * \pre N/A
 * \post N/A
 * \return int
 * \retval Index of data set
 * \retval SHORTCUT_ERROR_INVALID Invalid handle
 * \retval SHORTCUT_ERROR_MEMORY Out of memory
 * \see shortcut_icon_request_create
 */
extern int shortcut_icon_request_set_info(struct shortcut_icon *handle, const char *id, const char *type, const char *part, const char *data, const char *option, const char *subid);

/*!
 * \brief Send request to create an icon image
 * \remarks N/A
 * \details N/A
 * \param[in] handle Icon request handle
 * \param[in] size_type Size type to be created
 * \param[in] layout layout filename (edje filename)
 * \param[in] group group name
 * \param[in] outfile output image filename
 * \param[in] result_cb Result callback
 * \param[in] data Callback data
 * \pre N/A
 * \post N/A
 * \return int
 * \retval SHORTCUT_ERROR_INVALID Invalid parameters
 * \retval SHORTCUT_ERROR_MEMORY Out of memory
 * \retval SHORTCUT_ERROR_FAULT Failed to send a request
 * \retval SHORTCUT_SUCCESS Successfully sent
 * \see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_send(struct shortcut_icon *handle, int size_type, const char *layout, const char *group, const char *outfile, icon_request_cb_t result_cb, void *data);

/*!
 * \brief Destroy handle of creating shortcut icon request
 * \remarks N/A
 * \details N/A
 * \param[in] handle Shortcut request handle
 * \pre N/A
 * \post N/A
 * \return int
 * \retval SHORTCUT_ERROR_INVALID Invalid handle
 * \retval SHORTCUT_SUCCESS Successfully destroyed
 * \see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_destroy(struct shortcut_icon *handle);


/*!
 * \brief Set private data to the handle to carry it with a handle.
 * \remarks N/A
 * \details N/A
 * \param[in] handle Handle to be used for carrying a data
 * \param[in] data Private data
 * \pre N/A
 * \post N/A
 * \return int
 * \retval SHORTCUT_ERROR_INVALID Invalid handle
 * \retval SHORTCUT_SUCCESS Successfully done
 * \see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_set_data(struct shortcut_icon *handle, void *data);

/*!
 * \brief Get the private data from handle
 * \remarks N/A
 * \details N/A
 * \param[in] handle
 * \pre N/A
 * \post N/A
 * \return int
 * \retval NULL If there is no data
 * \retval pointer data pointer
 * \see shortcut_icon_request_set_data
 */
extern void *shortcut_icon_request_data(struct shortcut_icon *handle);

#ifdef __cplusplus
}
#endif

#endif
/* @}
 * End of a file 
 */
