cmake_minimum_required(VERSION 3.0)
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

execute_process(COMMAND "${PKG_CONFIG_EXECUTABLE}" hexchat-plugin --variable=hexchatlibdir
    RESULT_VARIABLE RES
    OUTPUT_VARIABLE HEXCHAT_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT ${RES} EQUAL 0)
    message(FATAL_ERROR "cannot get HexChat libdir")
endif()
install(TARGETS autoaway DESTINATION "${HEXCHAT_LIBDIR}")
