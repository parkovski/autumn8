cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "CMAKE_BUILD_TYPE not defined; defaulting to Debug")
  set(CMAKE_BUILD_TYPE Debug)
endif()

project(Autumn8 LANGUAGES C CXX)
set(A8_VERSION_MAJOR 0)
set(A8_VERSION_MINOR 1)
set(A8_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(A8_BUILD_TESTS_DEFAULT ON)
else()
  set(A8_BUILD_TESTS_DEFAULT OFF)
endif()
option(A8_BUILD_TESTS "Build tests" ${A8_BUILD_TESTS_DEFAULT})
option(A8_BUILD_APPS "Build apps" ON)
option(A8_BUILD_GUI "Build GUI targets" ON)

include(cmake/platform.cmake)

if(A8_GFXBACKEND STREQUAL Gtk)
  set(A8_GTK_VERSION 3 CACHE STRING "Gtk+ version")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

include(cmake/modules.cmake)

set(Autumn8_INCLUDE_DIRS
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)
include_directories(${Autumn8_INCLUDE_DIRS})

# TODO: Install? Build dir layout? debug/release?
cmake_path(SET A8_WEB_ASSET_PATH build/web)
cmake_path(ABSOLUTE_PATH A8_WEB_ASSET_PATH NORMALIZE)

include(cmake/app.cmake)

add_subdirectory(lib/main main)
add_subdirectory(lib/midi midi)
add_subdirectory(lib/util util)

if(A8_BUILD_GUI)
  message(STATUS "GUI targets are enabled")
  add_subdirectory(lib/gfx gfx)

  if(fpsdk_FOUND)
    message(STATUS "FL Studio plugin target is enabled")
    add_subdirectory(lib/flp flp)
  endif()
endif()

if(A8_BUILD_APPS)
  message(STATUS "App targets are enabled")
  add_subdirectory(app app)
endif()

if(A8_BUILD_TESTS)
  message(STATUS "Test targets are enabled")
  add_subdirectory(test)
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
  @ONLY
)
