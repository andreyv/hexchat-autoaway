cmake_minimum_required(VERSION 3.4)
project(hexchat-autoaway)

find_package(X11 REQUIRED)
if(NOT ${X11_Xscreensaver_FOUND})
    message(FATAL_ERROR "X11 Screen Saver extension library not found")
endif()
include_directories(SYSTEM "${X11_X11_INCLUDE_PATH}" "${X11_Xscreensaver_INCLUDE_PATH}")

find_package(GTK2 2.14 REQUIRED gtk)
include_directories(SYSTEM ${GTK2_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(HEXCHAT REQUIRED hexchat-plugin) # TODO: Use IMPORTED_TARGET with CMake 3.6+
include_directories(SYSTEM ${HEXCHAT_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wextra -pedantic ${CMAKE_C_FLAGS_RELEASE})
endif()

add_library(autoaway MODULE autoaway.c)
target_link_libraries(autoaway "${X11_X11_LIB}" "${X11_Xscreensaver_LIB}" ${GTK2_LIBRARIES} ${HEXCHAT_LIBRARIES})

pkg_get_variable(HEXCHAT_LIBDIR hexchat-plugin hexchatlibdir)

install(TARGETS autoaway DESTINATION "${HEXCHAT_LIBDIR}")
