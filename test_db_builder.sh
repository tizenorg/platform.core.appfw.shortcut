#!/bin/sh
#/*
# * Copyright (c) 2011 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# * http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#*/

DBFILE="/opt/dbspace/.shortcut_service.db"

APPID=(
"org.tizen.facebook"
"org.tizen.facebook"
"org.tizen.facebook"
"org.tizen.facebook"
"org.tizen.facebook"
)

ICON=(
""
""
""
""
""
)

NAME=(
"Friend's wall"
"Groups"
"Update status"
"Like by me"
"My wall"
)

KEY=(
"dynamicbox_shortcut_type"
"dynamicbox_shortcut_type"
"dynamicbox_shortcut_type"
"dynamicbox_shortcut_type"
"dynamicbox_shortcut_type"
)

VALUE=(
"shortcut_friends"
"shortcut_groups"
"shortcut_post"
"shortcut_like"
"shortcut_me"
)

CNT=0
ERR=0
MAX=5

sqlite3 $DBFILE "CREATE TABLE shortcut_service (id INTEGER PRIMARY KEY AUTOINCREMENT, appid TEXT, icon TEXT, name TEXT, extra_key TEXT, extra_data TEXT)"
sqlite3 $DBFILE "CREATE TABLE shortcut_name (id INTEGER, lang TEXT, name TEXT)"
while [ $CNT -lt $MAX ]
do
	echo "Insert a new record ('${APPID[$CNT]}', '${ICON[$CNT]}', '${NAME[$CNT]}', \"${KEY[$CNT]}\", \"${VALUE[$CNT]}\")"
	sqlite3 $DBFILE "INSERT INTO shortcut_service (appid, icon, name, extra_key, extra_data) VALUES ('${APPID[$CNT]}', '${ICON[$CNT]}', \"${NAME[$CNT]}\", \"${KEY[$CNT]}\", \"${VALUE[$CNT]}\")" 2>/dev/null
	if [ $? -ne 0 ]; then
		let ERR=$ERR+1
	fi
	ID=`sqlite3 $DBFILE "SELECT id FROM shortcut_service WHERE appid = \"${APPID[$CNT]}\" AND extra_key = \"${KEY[$CNT]}\" AND extra_data = \"${VALUE[$CNT]}\""`
	echo "Insert a name: \"${NAME[$CNT]}\""
	sqlite3 $DBFILE "INSERT INTO shortcut_name (id, lang, name) VALUES ('$ID', 'en-us', \"${NAME[CNT]}\")"
	let CNT=$CNT+1
done

echo "Error/Total: $ERR/$CNT"
