#!/bin/sh

DBFILE="/opt/dbspace/.shortcut_service.db"

PKGNAME=(
"com.samsung.music-player"
"com.samsung.music-player"
"com.samsung.music-player"
"com.samsung.gallery"
"com.samsung.gallery"
"com.samsung.contacts"
"com.samsung.contacts"
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

sqlite3 $DBFILE "CREATE TABLE shortcut_service (pkgname TEXT, name TEXT, service TEXT)"
while [ $CNT -lt 7 ]
do
	echo "Insert a new record ('${PKGNAME[$CNT]}', '${NAME[$CNT]}', '${SERVICE[$CNT]}')"
	sqlite3 $DBFILE "INSERT INTO shortcut_service (pkgname, name, service) VALUES ('${PKGNAME[$CNT]}', '${NAME[$CNT]}', '${SERVICE[$CNT]}')" 2>/dev/null
	if [ $? -ne 0 ]; then
		let ERR=$ERR+1
	fi
	let CNT=$CNT+1
done

echo "Error/Total: $ERR/$CNT"
