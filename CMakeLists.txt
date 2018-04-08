cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(Autumn8)
set(A8_VERSION_MAJOR 0)
set(A8_VERSION_MINOR 1)
set(A8_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif()

#
# External libraries
#
find_package(spdlog)
find_package(eigen3)
if(eigen3_FOUND)
  set(NANOGUI_EIGEN_INCLUDE_DIR ${eigen3_INCLUDE_DIR})
endif()

find_path(cppwinrt_INCLUDE_DIR winrt/Windows.System.h)
find_path(nanogui_DIR include/nanogui/nanogui.h HINTS "$ENV{nanogui_DIR}")
find_path(fpsdk_DIR source/c/fp_cplug.h HINTS "$ENV{fpsdk_DIR}")

#
# General
#
file(GLOB Autumn8_CORE_SOURCES src/core.*.cpp)
file(GLOB Autumn8_CMD_SOURCES src/cmd.*.cpp)
file(GLOB Autumn8_GUI_SOURCES src/gui.*.cpp)
file(GLOB Autumn8_STANDALONE_SOURCES src/standalone.*.cpp)
file(GLOB Autumn8_FL_SOURCES src/fl.*.cpp)

set(Autumn8_INCLUDE_PRIV $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set(Autumn8_INCLUDE_PUB
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(spdlog_NOTFOUND)
  message(STATUS "Logging disabled, install spdlog to enable")
else()
  set(HAS_SPDLOG 1)
  list(APPEND Autumn8_INCLUDE_PRIV ${spdlog_INCLUDE_DIR})
endif()

#
# Main library
#
add_library(Autumn8Core STATIC ${Autumn8_CORE_SOURCES})
target_include_directories(Autumn8Core PUBLIC
  $<BUILD_INTERFACE:${Autumn8_INCLUDE_PUB}>
  $<INSTALL_INTERFACE:include>)
target_include_directories(Autumn8Core PRIVATE ${Autumn8_INCLUDE_PRIV})

if(cppwinrt_INCLUDE_DIR_NOTFOUND)
  message(STATUS "UWP MIDI disabled, install cppwinrt to enable.")
else()
  set(HAS_CPPWINRT 1)
  target_sources(Autumn8Core PRIVATE src/midi.uwp.cpp)
  target_include_directories(Autumn8Core PRIVATE ${cppwinrt_INCLUDE_DIR})
endif()

#
# Command line interface
#
add_executable(a8 ${Autumn8_CMD_SOURCES})
target_link_libraries(a8 Autumn8Core)
target_include_directories(a8 PRIVATE ${Autumn8_INCLUDE_PRIV})

#
# Standalone GUI
#
if(nanogui_DIR_NOTFOUND)
  message(WARNING "GUI target disabled, set nanogui_DIR to enable.")
else()
  set(NANOGUI_BUILD_EXAMPLE OFF CACHE INTERNAL "NanoGUI examples")
  set(NANOGUI_BUILD_PYTHON OFF CACHE INTERNAL "NanoGUI Python integration")
  set(NANOGUI_INSTALL OFF CACHE INTERNAL "NanoGUI install")

  add_subdirectory(${nanogui_DIR} ${CMAKE_CURRENT_BINARY_DIR}/nanogui)
  get_directory_property(nanogui_INCLUDE_DIR DIRECTORY ${nanogui_DIR} INCLUDE_DIRECTORIES)

  add_executable(Autumn8 WIN32 ${Autumn8_STANDALONE_SOURCES} ${Autumn8_GUI_SOURCES})
  target_include_directories(Autumn8 PRIVATE
    ${Autumn8_INCLUDE_PRIV}
    ${Autumn8_INCLUDE_PUB}
    ${nanogui_INCLUDE_DIR}
    ${NANOGUI_EXTRA_INCS}
  )
  target_compile_definitions(Autumn8 PRIVATE ${NANOGUI_EXTRA_DEFS})
  target_link_libraries(Autumn8 Autumn8Core nanogui ${NANOGUI_EXTRA_LIBS})

  add_custom_command(TARGET Autumn8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:nanogui>"
      "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying nanogui.dll to main build directory."
  )

  message(STATUS "Standalone GUI target enabled.")

  #
  # FL plugin
  #
  if(fpsdk_DIR)
    file(GLOB fpsdk_SOURCES ${fpsdk_DIR}/source/c *.cpp)
    add_library(Autumn8FL SHARED ${Autumn8_FL_SOURCES} ${Autumn8_GUI_SOURCES} ${fpsdk_SOURCES})
    target_include_directories(Autumn8FL PRIVATE
      ${Autumn8_INCLUDE_PRIV}
      ${fpsdk_DIR}/source/c
      ${nanogui_INCLUDE_DIR}
      ${NANOGUI_EXTRA_INCS}
    )
    target_compile_definitions(Autumn8FL PRIVATE ${NANOGUI_EXTRA_DEFS})
    target_link_libraries(Autumn8FL Autumn8Core nanogui ${NANOGUI_EXTRA_LIBS})

    message(STATUS "FL plugin target enabled.")
  endif() # fpsdk
endif() # nanogui

configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
)
