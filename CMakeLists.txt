cmake_minimum_required(VERSION 3.4)
project(hexchat-autoaway)

add_library(autoaway MODULE autoaway.c)

find_package(X11 REQUIRED)
if(NOT ${X11_Xscreensaver_FOUND})
    message(FATAL_ERROR "X11 Screen Saver extension library not found")
endif()
include_directories("${X11_X11_INCLUDE_PATH}" "${X11_Xscreensaver_INCLUDE_PATH}")
target_link_libraries(autoaway "${X11_X11_LIB}" "${X11_Xscreensaver_LIB}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(HEXCHAT REQUIRED hexchat-plugin)
include_directories(${HEXCHAT_INCLUDE_DIRS})
target_link_libraries(autoaway ${HEXCHAT_LIBRARIES})

pkg_get_variable(HEXCHAT_LIBDIR hexchat-plugin hexchatlibdir)

install(TARGETS autoaway DESTINATION "${HEXCHAT_LIBDIR}")
