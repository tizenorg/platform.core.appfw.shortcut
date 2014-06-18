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

#if !defined(__SHORTCUT_INTERNAL_H__)
#define __SHORTCUT_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file shortcut_internal.h
 * @brief This file declares API of libshortcut library (platform only)
 */

/**
 * @addtogroup SHORTCUT_ICON_MODULE
 * @{
 */

/**
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
 * @retval SHORTCUT_ERROR_INVALID Already initialized
 * @retval SHORTCUT_ERROR_SUCCESS Successfully initialized
 * @see shortcut_icon_service_fini
 */
extern int shortcut_icon_service_init(int (*init_cb)(int status, void *data), void *data);

/**
 * @brief Finalizes the icon creation service.
 * @return int value
 * @retval SHORTCUT_SUCCESS Successfully initialized
 * @retval SHORTCUT_ERROR_INVALID icon service is not initialized
 * @see shortcut_icon_service_init
 */
extern int shortcut_icon_service_fini(void);

/**
 * @brief Creates a request object to create a new icon image.
 * @return struct shortcut_icon * value
 * @retval NULL If it fails to create a new handle
 * @retval pointer Handle address
 * @see shortcut_icon_request_destroy
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
 * @retval SHORTCUT_ERROR_INVALID Invalid handle
 * @retval SHORTCUT_ERROR_MEMORY Out of memory
 * @see shortcut_icon_request_create
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
 * @retval SHORTCUT_ERROR_INVALID Invalid parameters
 * @retval SHORTCUT_ERROR_MEMORY Out of memory
 * @retval SHORTCUT_ERROR_FAULT Failed to send a request
 * @retval SHORTCUT_SUCCESS Successfully sent
 * @see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_send(struct shortcut_icon *handle, int size_type, const char *layout, const char *group, const char *outfile, icon_request_cb_t result_cb, void *data);

/**
 * @brief Destroys handle of creating shortcut icon request.
 * @param[in] handle Shortcut request handle
 * @return int value
 * @retval SHORTCUT_ERROR_INVALID Invalid handle
 * @retval SHORTCUT_SUCCESS Successfully destroyed
 * @see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_destroy(struct shortcut_icon *handle);


/**
 * @brief Sets private data to the handle to carry it with a handle.
 * @param[in] handle Handle to be used for carrying a data
 * @param[in] data Private data
 * @return int value
 * @retval SHORTCUT_ERROR_INVALID Invalid handle
 * @retval SHORTCUT_SUCCESS Successfully done
 * @see shortcut_icon_service_fini
 */
extern int shortcut_icon_request_set_data(struct shortcut_icon *handle, void *data);

/**
 * @brief Gets the private data from handle.
 * @param[in] handle
 * @return int value
 * @retval NULL If there is no data
 * @retval pointer data pointer
 * @see shortcut_icon_request_set_data
 */
extern void *shortcut_icon_request_data(struct shortcut_icon *handle);

#ifdef __cplusplus
}
#endif

#endif
/* @}
 * End of a file 
 */
