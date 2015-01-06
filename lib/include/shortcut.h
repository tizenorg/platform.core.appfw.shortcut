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
 * @internal
 * @brief Called to the add_to_home request.
 * @details The homescreen should define a callback as this type and implement the service code
 *        for adding a new application shortcut.
 * @since_tizen 2.3
 * @param[in] appid The shortcut that is added for this package
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
typedef int (*request_cb_t)(const char *appid, const char *name, int type, const char *content_info, const char *icon, int pid, double period, int allow_duplicate, void *data);

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

	SHORTCUT_REMOVE		= 0x40000000,	/**< Remove a shortcut */
	LIVEBOX_REMOVE		= 0x80000000,	/**< Remove a livebox */

	LIVEBOX_TYPE_DEFAULT	  = 0x10000000,	/**< Type mask for the default livebox */
	LIVEBOX_TYPE_EASY_DEFAULT = 0x30000000,	/**< Type mask for the easy mode livebox */
	LIVEBOX_TYPE_1x1	  = 0x10010000,	/**< 1x1 */
	LIVEBOX_TYPE_2x1	  = 0x10020000,	/**< 2x1 */
	LIVEBOX_TYPE_2x2	  = 0x10040000,	/**< 2x2 */
	LIVEBOX_TYPE_4x1	  = 0x10080000,	/**< 4x1 */
	LIVEBOX_TYPE_4x2	  = 0x10100000,	/**< 4x2 */
	LIVEBOX_TYPE_4x3  	  = 0x10200000,	/**< 4x3 */
	LIVEBOX_TYPE_4x4	  = 0x10400000,	/**< 4x4 */
	LIVEBOX_TYPE_4x5	  = 0x11000000,	/**< 4x5 */
	LIVEBOX_TYPE_4x6	  = 0x12000000, /**< 4x6 */
	LIVEBOX_TYPE_EASY_1x1	  = 0x30010000,	/**< Easy mode 1x1 */
	LIVEBOX_TYPE_EASY_3x1	  = 0x30020000,	/**< Easy mode 3x2 */
	LIVEBOX_TYPE_EASY_3x3	  = 0x30040000,	/**< Easy mode 3x3 */
	LIVEBOX_TYPE_UNKNOWN	  = 0x1FFF0000,	/**< Error */
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
 * @retval true(1) If it is a livebox
 * @retval false(0) If it is not a livebox
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_LIVEBOX(type)	(!!((type) & 0x10000000))

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
 *
 * @internal
 *
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
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @param[in] request_cb The callback function pointer that is invoked when add_to_home is requested
 * @param[in] data The callback data to deliver to the callback function
 *
 * @return The return type (int)
 * @retval 0 Callback function is successfully registered
 * @retval <0 Failed to register the callback function for the request
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
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get a proper error code.
 * @param[in] appid The package name of the owner of this shortcut
 * @param[in] name The name of the created shortcut icon
 * @param[in] type The type of shortcuts (livebox or shortcut, and its size if it is for the livebox)
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
 * @privilege %http://tizen.org/privilege/shortcut
 *
 * @remarks If a homescreen does not support this feature, you will get a proper error code.
 * @param[in] appid The  package name
 * @param[in] cb The callback function to get the shortcut item information
 * @param[in] data The callback data that is used in the callback function
 *
 * @return The return type (int)
 * @retval @c N Number of items (call count of the callback function)
 * @retval #SHORTCUT_ERROR_FAULT Unrecoverable error
 * @retval #SHORTCUT_ERROR_IO_ERROR Unable to access the file or DB. Check your resource files
 *
 * @pre You have to prepare the callback function.
 *
 * @post You have to check the return status from the callback function which is passed by the argument.
 *
 * @see result_internal_cb_t
 */
extern int shortcut_get_list(const char *appid, int (*cb)(const char *appid, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data);

/**
 *
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
 * @param[in] type The type of shortcuts (livebox or shortcut, and its size if it is for the livebox)
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
 * 	add_to_home_livebox("org.tizen.gallery.livebox", "With friends",
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
extern int add_to_home_livebox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data);

struct shortcut_icon;

/**
 * @brief Called when send a request to create a icon snapshot image.
 * @details This callback will be called with its result.
 * @param[in] handle Handle of requestor
 * @param[in] ret status of request
 * @param[in] data Callback data
 * @return int result state of callback call
 * @retval 0 If it is successfully completed
 * @see shortcut_icon_request_send()
 */
typedef int (*icon_request_cb_t)(struct shortcut_icon *handle, int ret, void *data);

#define DEFAULT_ICON_PART		"icon"
#define DEFAULT_NAME_PART		"name"
#define SHORTCUT_ICON_TYPE_IMAGE	"image"
#define SHORTCUT_ICON_TYPE_TEXT		"text"
#define SHORTCUT_ICON_TYPE_SCRIPT	"script"

/**
 * @brief Initializes the icon creation service.
 * @param[in] init_cb Initialized result will be delievered via this callback
 * @param[in] data Callback data
 * @return int value
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Already initialized
 * @retval #SHORTCUT_ERROR_NONE Successfully initialized
 * @see shortcut_icon_service_fini()
 */
extern int shortcut_icon_service_init(int (*init_cb)(int status, void *data), void *data);

/**
 * @brief Finalizes the icon creation service.
 * @return int value
 * @retval #SHORTCUT_ERROR_NONE Successfully initialized
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER icon service is not initialized
 * @see shortcut_icon_service_init()
 */
extern int shortcut_icon_service_fini(void);

/**
 * @brief Creates a request object to create a new icon image.
 * @return struct shortcut_icon * value
 * @retval @c NULL If it fails to create a new handle
 * @retval pointer Handle address
 * @see shortcut_icon_request_destroy()
 */
extern struct shortcut_icon *shortcut_icon_request_create(void);

/**
 * @brief Sets information for creating icon image.
 * @param[in] handle Request handle
 * @param[in] id Target ID to be affected by this data
 * @param[in] type SHORTCUT_ICON_TYPE_IMAGE, SHORTCUT_ICON_TYPE_TEXT, SHORTCUT_ICON_TYPE_SCRIPT can be used
 * @param[in] part Target part to be affect by this data
 * @param[in] data type == IMAGE ? Image file path : type == TEXT ? text string : type == SCRIPT ? script file path : N/A
 * @param[in] option Image load option or group name of script file to be loaded
 * @param[in] subid ID for script. this ID will be used as "id"
 * @return int value
 * @retval index Index of data set
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Out of memory
 * @see shortcut_icon_request_create()
 */
extern int shortcut_icon_request_set_info(struct shortcut_icon *handle, const char *id, const char *type, const char *part, const char *data, const char *option, const char *subid);

/**
 * @brief Sends requests to create an icon image.
 * @param[in] handle Icon request handle
 * @param[in] size_type Size type to be created
 * @param[in] layout Layout filename (edje filename)
 * @param[in] group Group name
 * @param[in] outfile Output image filename
 * @param[in] result_cb Result callback
 * @param[in] data Callback data
 * @return int value
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #SHORTCUT_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SHORTCUT_ERROR_FAULT Failed to send a request
 * @retval #SHORTCUT_ERROR_NONE Successfully sent
 * @see shortcut_icon_service_fini()
 */
extern int shortcut_icon_request_send(struct shortcut_icon *handle, int size_type, const char *layout, const char *group, const char *outfile, icon_request_cb_t result_cb, void *data);

/**
 * @brief Destroys handle of creating shortcut icon request.
 * @param[in] handle Shortcut request handle
 * @return int value
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #SHORTCUT_ERROR_NONE Successfully destroyed
 * @see shortcut_icon_service_fini()
 */
extern int shortcut_icon_request_destroy(struct shortcut_icon *handle);


/**
 * @brief Sets private data to the handle to carry it with a handle.
 * @param[in] handle Handle to be used for carrying a data
 * @param[in] data Private data
 * @return int value
 * @retval #SHORTCUT_ERROR_INVALID_PARAMETER Invalid handle
 * @retval #SHORTCUT_ERROR_NONE Successfully done
 * @see shortcut_icon_service_fini()
 */
extern int shortcut_icon_request_set_data(struct shortcut_icon *handle, void *data);

/**
 * @brief Gets the private data from handle.
 * @param[in] handle
 * @return int value
 * @retval @c NULL If there is no data
 * @retval pointer data pointer
 * @see shortcut_icon_request_set_data()
 */
extern void *shortcut_icon_request_data(struct shortcut_icon *handle);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
