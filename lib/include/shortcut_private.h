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

#if !defined(FLOG)
#define DbgPrint(format, arg...)	SECURE_LOGD(format, ##arg)
#define ErrPrint(format, arg...)	SECURE_LOGE(format, ##arg)
#else
extern FILE *__file_log_fp;
#define DbgPrint(format, arg...) do { fprintf(__file_log_fp, "[LOG] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)

#define ErrPrint(format, arg...) do { fprintf(__file_log_fp, "[ERR] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#endif

#if !defined(EAPI)
#define EAPI __attribute__((visibility("default")))
#endif

#if !defined(VCONFKEY_MASTER_STARTED)
#define VCONFKEY_MASTER_STARTED	"memory/data-provider-master/started"
#endif

#define DEFAULT_ICON_LAYOUT ""
#define DEFAULT_ICON_GROUP ""

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
 *
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
5 * 	add_to_home_remove_shortcut("org.tizen.gallery.dynamicbox", "With friends",
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
extern int add_to_home_remove_shortcut(const char *appid, const char *name, const char *content_info, result_internal_cb_t result_cb, void *data);

/**
 *
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
 * 	add_to_home_remove_dynamicbox("org.tizen.gallery.dynamicbox", "With friends", result_cb, NULL);
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
extern int add_to_home_remove_dynamicbox(const char *appid, const char *name, result_internal_cb_t result_cb, void *data);

/* End of a file */
