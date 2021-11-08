cmake_minimum_required(VERSION 3.21.1 FATAL_ERROR)

project(Autumn8 LANGUAGES C CXX)
set(A8_VERSION_MAJOR 0)
set(A8_VERSION_MINOR 1)
set(A8_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Deb")
  set(A8_TEST_DEFAULT ON)
else()
  set(A8_TEST_DEFAULT OFF)
endif()
option(A8_BUILD_TESTS "Build tests" ${A8_TEST_DEFAULT})
option(A8_BUILD_TOOLS "Build tools" ON)
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

#
# Targets
#
set(Autumn8_INCLUDE_DIRS
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

#
# External libraries
#
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(SndFile REQUIRED) # SndFile::sndfile
find_package(Boost REQUIRED) # signals2

find_package(uWebSockets QUIET)

if(A8_BUILD_GUI AND A8_FLSTUDIO_PLATFORM)
  find_package(fpsdk QUIET)
endif()

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  "${PROJECT_SOURCE_DIR}/contrib/moodycamel/include"
)

include_directories(${Autumn8_INCLUDE_DIRS})

add_subdirectory(src/main main)
add_subdirectory(src/midi midi)
add_subdirectory(src/util util)

if(A8_BUILD_GUI)
  message(STATUS "GUI targets are enabled")
  add_subdirectory(src/gfx gfx)

  if(fpsdk_FOUND)
    message(STATUS "FL Studio plugin target is enabled")
    add_subdirectory(src/flp flp)
  endif()
endif()

if(A8_BUILD_TOOLS)
  message(STATUS "Tools targets are enabled")
  add_subdirectory(src/tools tools)
endif()

if(A8_BUILD_TESTS)
  message(STATUS "Test targets are enabled")
  add_subdirectory(test)
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
)
