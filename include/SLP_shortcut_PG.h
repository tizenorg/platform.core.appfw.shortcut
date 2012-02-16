/*
 * libslp-shortcut-0
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Contact: Sung-jae Park <nicesj.park@samsung.com>, Youngjoo Park <yjoo93.park@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS ("Confidential Information").
 * You shall not disclose such Confidential Information and shall use it only in accordance with the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
 * SAMSUNG make no representations or warranties about the suitability of the software, either express or implied, including but not limited to the implied warranties of merchantability, fitness for a particular purpose, or non-infringement.
 * SAMSUNG shall not be liable for any damages suffered by licensee as a result of using, modifying or distributing this software or its derivatives.
 *
 *
 */

/**
 *  @ingroup SLP_PG
 *  @defgroup SHORTCUT_PG Add to home (shortcut)
 *  @{

 <h1 class="pg">Introduction</h1>
 Shortcut is a communication interfaces for homescreen and applications. To enable the add_to_home feature. Shortcut(libslp-shortcut) uses socket for IPC.

 @image html SLP_Shortcut_PG_image01.png

 <h2 class="pg">Properties</h2>
- Types of shortcut
- Header File : shortcut.h 

<h1 class="pg">Programming Guide</h1>

<h2 class="pg">Types of shortcut</h1>
Shortcut defines 3 kinds of types.

<h3 class="pg">1.	SHORTCUT_PACKAGE</h1>
We can use this for adding a package shortcut with additional information. <br/>
@code
@endcode

<h3 class="pg">2.	SHORTCUT_DATA</h1>
@code
@endcode

<h3 class="pg">3.	SHORTCUT_FILE</h1>
@code
@endcode

<h2 class="pg">Error code</h1>
<h3 class="pg">1.	-EINVAL</h1>
<h3 class="pg">1.	-EFAULT</h1>
@code
@endcode
*/

/**
@}
*/
