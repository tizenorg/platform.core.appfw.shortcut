PROJECT(shortcut_service_register C)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/bin/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(bin_pkgs REQUIRED
	dlog
	sqlite3
	libxml-2.0
	db-util
)

FOREACH(flag ${bin_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DLOG_TAG=\"shortcut_service_register\"")

ADD_EXECUTABLE(${PROJECT_NAME}
	src/service_register.c
	src/dlist.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${bin_pkgs_LDFLAGS})

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin)
