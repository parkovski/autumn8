cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Autumn8)
set(A8_VERSION_MAJOR 0)
set(A8_VERSION_MINOR 1)
set(A8_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

option(A8_GUI_ENABLED "Enable GUI targets" ON)
option(A8_BUILD_TOOLS "Build tools" ON)
option(A8_BUILD_TESTS "Build tests" OFF)
option(A8_BUILD_DOTNET "Build the .NET projects" OFF)

if(MSVC)
#   add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(-diagnostics:caret)
endif()

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
endif()

#
# External libraries
#
find_package(spdlog REQUIRED)
find_package(fmt CONFIG REQUIRED)
# find_package(Sol2 REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(nlohmann-json REQUIRED)
find_package(ffts REQUIRED)
# find_package(RxCpp REQUIRED)
# find_package(cereal CONFIG REQUIRED)
# cpprestsdk, uvw, evpp, GameNetworkingSockets, kcp, enet, boost asio/beast?
find_package(Boost 1.67 REQUIRED
#  COMPONENTS algorithm geometry signals2
)

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  ${PROJECT_SOURCE_DIR}/contrib/moodycamel/include
)

#
# Targets
#
set(Autumn8_INCLUDE_DIRS
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)
include_directories(${Autumn8_INCLUDE_DIRS})
link_libraries(spdlog::spdlog fmt::fmt-header-only)

add_subdirectory(src/util util)
add_subdirectory(src/script script)
add_subdirectory(src/main main)
add_subdirectory(src/midi midi)
add_subdirectory(src/audio audio)
add_subdirectory(src/uiauto uiauto)

if(A8_GUI_ENABLED)
  find_package(bgfx REQUIRED)
  find_package(NanoVG_bgfx REQUIRED)
  find_package(Yoga REQUIRED)
  find_package(fpsdk)

  message(STATUS "GUI targets are enabled.")
  add_subdirectory(src/gfx gfx)

  if(fpsdk_FOUND)
    message(STATUS "FL Studio plugin target is enabled.")
    add_subdirectory(src/fls fls)
  endif()
else()
  message(STATUS "GUI targets are disabled.")
endif()

if(A8_BUILD_TOOLS)
  add_subdirectory(src/tools tools)
endif()

if(A8_BUILD_TESTS)
  find_package(catch2 CONFIG REQUIRED)
  add_subdirectory(test)
endif()

if(A8_BUILD_DOTNET)
  include(cmake/DotNetBuild.cmake)
  # TODO...
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/a8config.h
)
