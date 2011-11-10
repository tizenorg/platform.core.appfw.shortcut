/*
 * shortcut
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

/**
 * @addtogroup APPLICATION_FRAMEWORK
 * @{
 */

/**
 * @defgroup SHORTCUT Add to home (shortcut)
 * @author Sung-jae Park <nicesj.park@samsung.com>
 * @version 0.1
 * @brief To support the add_to_home feature. Two types of APIs are supported.
 *        One for the homescreen developers.
 *        The others for the application developers who should to implement the add_to_home feature.
 */


/**
 * @brief This function prototype is used to define a callback function for the add_to_home reqeust.
 *        The homescreen should to define a callback as this type and implementing the service code
 *        for adding a new application shortcut.
 * @param[in] pkgname Shortcut is added for this package.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type 3 kinds of types are defined.
 * @param[in] content_info Specific information for creating a new shortcut.
 * @param[in] icon Absolute path of an icon file for this shortcut.
 * @param[in] pid Process ID of who request add_to_home.
 * @param[in] data Callback data.
 * @return int Developer should to returns the result of handling shortcut creation request.
 *             Returns 0, if succeed to handles the add_to_home request, or returns proper errno.
 * @see shortcut_set_request_cb
 * @pre None
 * @post None
 * @remarks None
 */
typedef int (*request_cb_t)(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, int pid, void *data);

/**
 * @brief This function prototype is used to define for receiving the result of add_to_home.
 * @param[in] ret Result value, it could be 0 if succeed to add a shortcut, or errno.
 * @param[in] pid Process ID of who handles this add_to_home request.
 * @param[in] data Callback data.
 * @return int Returns 0, if there is no error or returns errno.
 * @see shortcut_add_to_home()
 * @pre None
 * @post None
 * @remarks None
 */
typedef int (*result_cb_t)(int ret, int pid, void *data);

/**
 * @brief Basically, three types of shortcut is defined.
 *        Every homescreen developer should to support these types of shortcut.
 *        Or returns proper errno to figure out why the application failed to add a shortcut.
 *        SHORTCUT_PACKAGE is used for adding a package itself as a shortcut
 *        SHORTCUT_DATA is used for adding a shortcut for "content" data.
 *        SHORTCUT_FILE is used for adding a shortcut for "file".
 */
enum {
	SHORTCUT_PACKAGE = 0x0, /**< Launch the package using given pakcage name. */
	SHORTCUT_DATA = 0x01, /**< Launch the related package with given data(content_info). */
	SHORTCUT_FILE = 0x02, /** < Launch the related package with given filename(content_info). */
};

/**
 * @brief Homescreen should to use this function to service the shortcut creating request.
 * @param[in] request_cb Callback function pointer which will be invoked when add_to_home is requested.
 * @param[in] data Callback data to deliver to the callback function.
 * @return void There is no return value.
 * @see request_cb_t
 * @pre None
 * @post None
 * @remarks None
 * @par Example
 * @code
 *
 * static int request_cb(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, int pid, void *data)
 * {
 * 	printf("Package name: %s\n", pkgname);
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
extern void shortcut_set_request_cb(request_cb_t request_cb, void *data);

/**
 * @brief The application, which supporting the add_to_home feature, should to invoke this.
 * @param[in] pkgname Package name of owner of this shortcut.
 * @param[in] name Name for created shortcut icon.
 * @param[in] type 3 kinds of types are defined.
 * @param[in] content_info Specific information for delivering to the creating shortcut.
 * @param[in] icon Absolute path of an icon file
 * @param[in] result_cb Callback function pointer which will be invoked after add_to_home request.
 * @param[in] data Callback data to deliver to the callback function.
 * @return int Returns 0 if it succeed to send the request, or returns proper errno.
 * @see result_cb_t
 * @pre None
 * @post None
 * @remarks None
 * @par Example
 * @code
 * static int result_cb(int ret, int pid, void *data)
 * {
 * 	return 0;
 * }
 *
 * static int app_create(void *data)
 * {
 * 	shortcut_add_to_home("com.samsung.gallery", "With friends",
 * 					SHORTCUT_DATA, "gallery:0000-0000",
 * 					"/opt/media/Pictures/Friends.jpg", result_cb, NULL);
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
extern int shortcut_add_to_home(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, result_cb_t result_cb, void *data);

#endif
/* @}
 * End of a file 
 */
