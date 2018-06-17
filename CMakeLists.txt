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

option(WITHOUT_GUI "Don't add GUI targets" OFF)

if(MSVC)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(-diagnostics:caret)
endif()

if(APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(A8_TARGET "macOS")
  elseif(CMAKE_SYSTEM_NAME MATCHES "iOS" OR CMAKE_SYSTEM_NAME MATCHES "iPhone OS")
    set(A8_TARGET "iOS")
  else()
    message(FATAL_ERROR "Target ${CMAKE_SYSTEM_NAME} is not supported.")
  endif()
elseif(WIN32)
  set(A8_TARGET "Windows")
elseif(WINDOWS_STORE)
  set(A8_TARGET "UWP")
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
  set(A8_TARGET "Android")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(A8_TARGET "Linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(A8_TARGET "FreeBSD")
else()
  message(FATAL_ERROR "Target ${CMAKE_SYSTEM_NAME} is not supported.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR}/bin/minsizerel)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin/optdebug)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO})

#
# External libraries
#
find_package(spdlog CONFIG)
find_package(fmt REQUIRED)
find_package(cereal REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(catch2)

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  ${PROJECT_SOURCE_DIR}/contrib/moodycamel/include
)

if(NOT WITHOUT_GUI)
  find_package(imgui REQUIRED)
  find_package(bgfx REQUIRED)
  find_package(NanoVG REQUIRED)
  find_package(Yoga REQUIRED)
  find_package(fpsdk)

  set(GUI_TARGETS_OK 1)
endif() # NOT WITHOUT_GUI

if(spdlog_FOUND)
  set(HAS_SPDLOG 1)
  link_libraries(spdlog::spdlog fmt::fmt)
else()
  message("Logging disabled, install spdlog to enable")
endif()

#
# Targets
#
set(Autumn8_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
include_directories(${Autumn8_INCLUDE_DIRS})

add_subdirectory(src/core core)

if(GUI_TARGETS_OK)
  message(STATUS "GUI targets are enabled.")
  add_subdirectory(src/gfx gfx)

  if(fpsdk_FOUND)
    message(STATUS "FL Studio plugin target is enabled.")
    add_subdirectory(src/flplugin flplugin)
  endif()
else()
  message(STATUS "GUI targets are disabled.")
endif()

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/a8config.h
)
