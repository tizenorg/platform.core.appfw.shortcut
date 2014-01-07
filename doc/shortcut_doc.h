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

#ifndef __SHORTCUT_DOC_H__
#define __SHORTCUT_DOC_H__

/**
 * @defgroup SHORTCUT_MODULE Shortcut
 * @brief To enhance the Add to home feature. Two types of API set are supported.
 *   - One for the homescreen developers.
 *   - The others for the application developers who should implement the Add to home feature.
 * @ingroup CORE_LIB_GROUP 
 * @section SHORTCUT_MODULE_HEADER Required Header
 *   \#include <shortcut.h>
 * @section SHORTCUT_MODULE_OVERVIEW Overview
<H1>1. Shortcut</H1>
Tizen is supporting the "add shortcut or add to home" for various applications.
Developers may use the shortcut library (libshortcut) to implement features sending (applications) and receiving (possibly home screen) shortcuts.
If home screen implements the shortcut service using the library, the applications are good to go for adding their shortcuts to the home screen(, and vice versa.)

<H1>2. How to add a new shortcut to the home screen</H1>
<H2>2.1 Add to home (From the app to home)</H2>

The followings are two separate APIs to support  "add to home" feature. 
 
\code
typedef int (*result_cb_t)(int ret, int pid, void *data)

extern int add_to_home_shortcut(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, result_cb_t result_cb, void *data)
 
extern int add_to_home_livebox(const char *pkgname, const char *name, int type, const char *content, const char *icon, double period, result_cb_t result_cb, void *data)
\endcode

Currently, our home screen can contain two different types of contents (that are pure shortcuts and liveboxes).

To add a pure shortcut i.e., simply for launching an app, developers can use "add_to_home_shortcut" API to deliver their shortcuts to a home screen.

If your application supports our livebox service and implments livebox type shortcut, then you can use "add_to_home_livebox" API to have a home screen add the livebox on its screen.

<TABLE>
<TR><TH>Parameters</TH><TH>Comment</TH></TR>
<TR><TD>pkgname</TD><TD>Package name</TD></TR>
<TR><TD>name</TD><TD>Application name wich will be displayed on the screen</TD></TR>
<TR><TD>type</TD><TD>Basically it describes launching options whether to use a package name or URI. LAUNCH_BY_PACKAGE or LAUNCH_BY_URI</TD></TR>
<TR><TD>content</TD><TD>
Application data used for creating a pure shortcut or creating a livebox

    Shortcut

1. if the type is Launch by package: None

2. if the type is Launch by URI: put the URI in the content

    Livebox: any data necessary to create a livebox. Basically, it will passed to the livebox plug-in's create function.
</TD></TR>
<TR><TD>icon</TD><TD>Absolute path to the icon file, If you set this "NULL", the home screen will use the deafult icon file (but it is depends on the homescreen implementations)</TD></TR>
<TR><TD>period</TD><TD>&lt;Only for livebox> Update period. The period must be greater than 0.0f</TD></TR>
<TR><TD>result_cb</TD><TD>Result callback. The callback will be called after a shortcut or livebox has been added. Don't forget to check the return value.</TD></TR>
<TR><TD>data</TD><TD>Callback data</TD></TR>
</TABLE>

<H3>2.1.1 Supported types</H3>
\snippet lib/include/shortcut.h Enumeration values for type of shortcuts

<H2>2.2 Add shortcut (Home screen retrieves shortcuts from app)</H2>
"Add shortcut " service enables home screen to retrieve all shortcuts that applications promised to support and request an app to send their shortcuts to home directly, as opposed to "add to home".

It is initiated by home screen as opposed to "add to home" which is initiated by an application."Add shortcut " service enables home screen to retrieve all shortcuts that applications promised to support and request an app to send their shortcuts to home directly, as opposed to "add to home".

It is initiated by home screen as opposed to "add to home" which is initiated by an application.

<H3>2.2.1 Build the shortcut list DB</H3>
\image html BuildShortcutList.png

To recognize how many and what kinds of shortcuts have been registerred, each application who wants to support "add shortcut" service needs to install the XML file that defines "shortcut" information.
The following table describes the format and information necessary to define the shortcuts application can support.
Then, the information will be shown and used in home screen when a user selects "add shortcut" service.

<TABLE>
<TH><TD>Syntax of the shortcut manifest file</TD></TH>
<TR><TD>
\code
<manifest xmlns="http://tizen.org/ns/packages" package="com.samsung.APP_PKGNAME">
...
    <shortcut-list>
        <shortcut appid="com.samsung.myapp" extra_key="key_string" extra_data="data_string_for_key">
           <icon>/opt/share/icons/default/small/com.samsung.myapp-shortcut.png</icon>
           <label>Default name</label>
           <label xml:lang="en-us">Name en</label>
           <label xml:lang="nl-nl">Name nl</label>
           <label xml:lang="de-de">Name de</label>
           <label xml:lang="zh-hk">Name hk</label>
           <label xml:lang="zh-cn">Name cn</label>
           <label xml:lang="ru-ru">Name ru</label>
           <label xml:lang="zh-tw">Name tw</label>
           <label xml:lang="ja-jp">Name jp</label>
           <label xml:lang="es-es">Name es</label>
           <label xml:lang="el-gr">Name gr</label>
           <label xml:lang="it-it">Name it</label>
           <label xml:lang="tr-tr">Name tr</label>
           <label xml:lang="pt-pt">Name pt</label>
           <label xml:lang="fr-fr">Name fr</label>
           <label xml:lang="ko-kr">Name kr</label>
        </shortcut>
        <shortcut appid="com.samsung.myapp" extra_key="key_string" extra_data="data_string_for_key">
           <label>Default name</label>
        </shortcut>
        ...
        <shortcut appid="com.samsung.myapp" extra_key="key_string" extra_data="data_string_for_key">
           <label>Default name</label>
           <icon>/opt/share/icons/default/small/com.samsung.myapp2.png</icon>
        </shortcut>
    </shortcut-list>
...
</manifest>
\endcode
</TD></TR>
</TABLE>

Install this XML file to /opt/share/packages/YOUR_PKGNAME.xml (Note: After manifest has been applied to the binary, simply copy the content to your manifest file.)
Shortcut listing application will list up these items on its screen and each item will be displayed using the string in label field.
When selected, it will launch the selected app using app service with "appid" and a bundle holding data in the pair of extra_key and extra-data fields.
Obviously, icon will be used to display visual information for given shortcut type.(that said the icon could be other than the application's default icon)

<H3>2.2.2 Jump to the APP</H3>
\image html JumpToApp.png

Shortcut listing application will launch your application using app-svc with package name and param attributes.
It will use the pkgname and param attrbute to launch your application.
Shortcut list view will launch your "[App] Shortcut list" using followed code.

<TABLE>
<TH><TD>Launch your app from shortcut list viewer</TD></TH>
<TR><TD>
\code
service_h service;
service_create(&service);
service_set_package(service, APPID); /* <shortcut appid="APPID" ...> */
service_add_extra_data(service, EXTRA_KEY, EXTRA_DATA); /* <shortcut extra_key="EXTRA_KEY" extra_data="EXTRA_DATA" ...> */
ret = service_send_launch_request(service, NULL, NULL);
if (ret ...) { }
service_destroy(service);
\endcode
</TD></TR>
</TABLE>
When your app is launched, the app should send a selected item as a shortcut or livebox to the home screen using "add_to_home" series functions mentioned above.

<H3>2.2.3 What each app has to do</H3>
You can implement your shortcut list view using App or UG.

Who is going to handle the shortcut

<UL>
<LI>Handled by App
	<LI>App should prepare a shortcut-add view as guided</LI>
</LI>
<LI>Handeld by UG
	<LI>UG should be launched as an app and provide the shortcut-add view</LI>
</LI>
</UL>

In your shortcut list view, you just call the "add_to_home_shortcut" or "add_to_home_livebox" which are described in the section 2.1

<H4>2.2.3.1 Handled by App</H4>
\image html ShortcutApp.png
When your application is launched by the shortcut list application (displayed on the left most of above figure).
Your application should go back to the normal view when you receive the PAUSE event.
If you didn't change the view of your application, the user will see this shortcut list view again even if the user
launches your application from the app-tray(or homescreen).
So you have to change the view from the shortcut list to the normal(or previous) view when you get the PAUSE event.

<H4>2.2.3.2 Handled by UG</H4>
\image html ShortcutUG.png
In this case, the Shortcut List application will launch your UG as a process.
When you receive PAUSE event, or need to change to other view (not in the same UG), you should destroy current UG.
If you didn't destroy it, it will be reside on the process list. and it will not be destroyed automatically.

Currently, UG container process only supporting the multiple instance for a process.
So if the user tries to add a new shortcut again from the shortcut list application, your UG will be launched again if you didn't
terminate previous UG process (when you got PAUSE event).

<H1>3. What the home screen should do</H1>
\code
typedef int (*request_cb_t)(const char *pkgname, const char *name, int type, const char *content_info, const char *icon, int pid, double period, void *data)
extern int shortcut_set_request_cb(request_cb_t request_cb, void *data)
\endcode

<TABLE>
<TR><TH>Parameter</TH><TH>Comment</TH></TR>
<TR><TD>pkgname</TD><TD>Package name to be added</TD></TR>
<TR><TD>name</TD><TD>Application name to be displayed on the screen</TD></TR>
<TR><TD>type</TD><TD>LAUNCH_BY_PACKAGE or LAUNCH_BY_URI</TD></TR>
<TR><TD>content_info</TD><TD>Used for the livebox, or homescreen by itself if it required.</TD></TR>
<TR><TD>icon</TD><TD>Absolute path of the icon file. (If it is not exists, the homescreen can use the deafult icon file)</TD></TR>
<TR><TD>pid</TD><TD>Reuquestor's Process ID</TD></TR>
<TR><TD>period</TD><TD>Update period only for the livebox</TD></TR>
<TR><TD>data</TD><TD>Callback data</TD></TR>
</TABLE>

<H1>4. To list up shortcuts registred in the device</H1>
<TABLE>
<TR><TH>shortcut-list viewer will launch your app by this way</TH></TR>
<TR><TD>
\code
int shortcut_get_list(const char *pkgname, int (*cb)(const char *pkgname, const char *icon, const char *name, const char *extra_key, const char *extra_data, void *data), void *data)
\endcode
</TD></TR>
</TABLE>

If you specified the "pkgname", this API will only gathering the given Package's shortcut list.
If you set is to NULL, this API will gathering all shortcuts.
Every shortcut item will be passed via "cb" callback function. so it will be invoked N times if the number of registered shortcut item is N.
pkgname and name and param is described in the XML file of each application package.
It will returns the number of shortcut items, or return <0 as an error value.

-EIO : failed to access shortcut list DB
> 0 : Number of shortcut items (count of callback function calling)
 *
 */

#endif /* __SHORTCUT_DOC_H__ */
