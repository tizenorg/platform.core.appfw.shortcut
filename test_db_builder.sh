#!/bin/sh

DBFILE="/opt/dbspace/.shortcut_service.db"

APPID=(
"com.samsung.music-player"
"com.samsung.music-player"
"com.samsung.music-player"
"com.samsung.gallery"
"com.samsung.gallery"
"com.samsung.contacts"
"com.samsung.contacts"
)

ICON=(
"com.samsung.music-icon.png"
"com.samsung.music-icon.png"
"com.samsung.music-icon.png"
"com.samsung.gallery-icon.png"
"com.samsung.gallery-icon.png"
"com.samsung.contacts-icon.png"
"com.samsung.contacts-icon.png"
)

NAME=(
"My favorite songs"
"Albums"
"Artists"
"My family"
"2012 Travel"
"Best friends"
"Coworkers"
)

SERVICE=(
"favorite_song"
"album"
"artist"
"family"
"travel"
"friends"
"coworker"
)

CNT=0
ERR=0

sqlite3 $DBFILE "CREATE TABLE shortcut_service (id INTEGER PRIMARY KEY AUTOINCREMENT, appid TEXT, icon TEXT, name TEXT, extra_key TEXT, extra_data TEXT)"
sqlite3 $DBFILE "CREATE TABLE shortcut_name (id INTEGER, lang TEXT, name TEXT)"
while [ $CNT -lt 7 ]
do
	echo "Insert a new record ('${APPID[$CNT]}', '${ICON[$CNT]}', '${NAME[$CNT]}', '${SERVICE[$CNT]}')"
	sqlite3 $DBFILE "INSERT INTO shortcut_service (appid, icon, name, extra_key, extra_data) VALUES ('${APPID[$CNT]}', '${ICON[$CNT]}', '${NAME[$CNT]}', 'SHORTCUT', '${SERVICE[$CNT]}')" 2>/dev/null
	if [ $? -ne 0 ]; then
		let ERR=$ERR+1
	fi
	ID=`sqlite3 $DBFILE "SELECT id FROM shortcut_service WHERE appid = \"${APPID[$CND]}\" AND extra_key = \"SHORTCUT\" AND extra_data = \"${SERVICE[$CNT]}\""`
	echo "Insert a name: '${NAME[$CNT]} en'"
	sqlite3 $DBFILE "INSERT INTO shortcut_name (id, lang, name) VALUES ('$ID', 'en-us', '${NAME[CNT]} en')"
	let CNT=$CNT+1
done

echo "Error/Total: $ERR/$CNT"
