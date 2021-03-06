CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(shortcut C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${exec_prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include/${PROJECT_NAME}")
SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.0.1")

set(CMAKE_SKIP_BUILD_RPATH true)

SET(SRCS src/main.c src/secom_socket.c)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(glib_pkg REQUIRED gobject-2.0)
pkg_check_modules(pkgs REQUIRED
	glib-2.0
	dlog
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

#ADD_DEFINITIONS("-Werror")
#ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wextra")
#ADD_DEFINITIONS("-ansi")
#ADD_DEFINITIONS("-pedantic")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DLOG_TAG=\"${PROJECT_NAME}\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${VERSION})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

CONFIGURE_FILE(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.pc")

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/include/shortcut.h DESTINATION include/${PROJECT_NAME})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/include/SLP_shortcut_PG.h DESTINATION include/${PROJECT_NAME})
