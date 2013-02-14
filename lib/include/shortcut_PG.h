/*
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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

/**
 *  @ingroup SLP_PG
 *  @defgroup SHORTCUT_PG Add to home (shortcut)
 *  @{

 <h1 class="pg">Introduction</h1>
 Shortcut is a communication interfaces for homescreen and applications. To enable the add_to_home feature. Shortcut(libshortcut) uses socket for IPC.

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
