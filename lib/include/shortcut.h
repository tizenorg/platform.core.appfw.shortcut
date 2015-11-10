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
 * @brief Definition for a macro to check type.
 * @since_tizen 2.3
 * @param[in] type The type of box
 * @return bool
 * @retval true(1) If it is a dynamicbox
 * @retval false(0) If it is not a dynamicbox
 * @see shortcut_type
 */
#define ADD_TO_HOME_IS_DYNAMICBOX(type)	(!!((type) & 0x10000000))

/* DEPRECATED API */
extern int add_to_home_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon, int allow_duplicate, result_internal_cb_t result_cb, void *data) __attribute__((deprecated));
extern int add_to_home_dynamicbox(const char *appid, const char *name, int type, const char *content, const char *icon, double period, int allow_duplicate, result_internal_cb_t result_cb, void *data) __attribute__((deprecated));

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
