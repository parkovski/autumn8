cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Autumn8 LANGUAGES C CXX)
set(A8_VERSION_MAJOR 0)
set(A8_VERSION_MINOR 1)
set(A8_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# option(A8_BUILD_TESTS "Build tests" OFF)

if(APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(A8_PLATFORM "macOS")
    set(A8_GFXBACKEND "Cocoa")
  elseif(CMAKE_SYSTEM_NAME MATCHES "iOS" OR CMAKE_SYSTEM_NAME MATCHES "iPhone OS")
    set(A8_PLATFORM "iOS")
    set(A8_GFXBACKEND "CocoaTouch")
  else()
    message(FATAL_ERROR "Target ${CMAKE_SYSTEM_NAME} is not supported.")
  endif()
elseif(WIN32)
  set(A8_PLATFORM "Windows")
  set(A8_GFXBACKEND "Win32")
elseif(WINDOWS_STORE)
  set(A8_PLATFORM "UWP")
  set(A8_GFXBACKEND "Win32")
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
  set(A8_PLATFORM "Android")
  set(A8_GFXBACKEND "Android")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(A8_PLATFORM "Linux")
  set(A8_GFXBACKEND "Gtk")
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(A8_PLATFORM "FreeBSD")
  set(A8_GFXBACKEND "Gtk")
else()
  message(FATAL_ERROR "Target ${CMAKE_SYSTEM_NAME} is not supported.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

#
# External libraries
#
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(bgfx QUIET)
if(bgfx_FOUND)
  get_target_property(bx_INCLUDE_DIRS bgfx::bx INTERFACE_INCLUDE_DIRECTORIES)
  if(MSVC)
    set_target_properties(bgfx::bx PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${bx_INCLUDE_DIRS};${bx_INCLUDE_DIRS}/compat/msvc")
  endif()
  find_package(NanoVG_bgfx REQUIRED)
  set(A8_HAS_GUI ON)
  message(STATUS "GUI targets are enabled.")
  add_subdirectory(src/gfx gfx)

  # find_package(fpsdk)
  # if(fpsdk_FOUND)
  #   message(STATUS "FL Studio plugin target is enabled.")
  #   add_subdirectory(src/fls fls)
  # endif()
else()
  set(A8_HAS_GUI OFF)
  message(STATUS "GUI targets are disabled.")
endif()

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  "${PROJECT_SOURCE_DIR}/contrib/moodycamel/include"
)

#
# Targets
#
set(Autumn8_INCLUDE_DIRS
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)
include_directories(${Autumn8_INCLUDE_DIRS})
link_libraries(spdlog::spdlog_header_only fmt::fmt-header-only)

add_subdirectory(src/main main)
add_subdirectory(src/midi midi)
add_subdirectory(src/tools tools)
add_subdirectory(src/util util)

if(A8_BUILD_TESTS)
  add_subdirectory(test)
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
)
