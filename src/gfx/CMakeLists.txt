project(Autumn8Gfx)

set(SOURCES window.cpp)

add_library(gfx ${SOURCES})
set_target_properties(gfx PROPERTIES OUTPUT_NAME a8gfx)
target_link_libraries(gfx bgfx fmt::fmt cereal NanoVG Yoga concurrentqueue)

add_executable(guidriver WIN32 MACOSX_BUNDLE gfxmain.cpp)
set_target_properties(guidriver PROPERTIES OUTPUT_NAME Autumn8)
target_link_libraries(guidriver gfx bgfx concurrentqueue)

if(${A8_TARGET} STREQUAL "Windows")
  target_sources(gfx PRIVATE platform/windows/messageloop.cpp)
  target_sources(guidriver PRIVATE
    platform/windows/winmain.cpp
    ${CMAKE_SOURCE_DIR}/res/Autumn8.rc
  )
  set(CMAKE_EXE_LINKER_FLAGS /MANIFEST:NO)
  set(WINDOWS_RESOURCES ${CMAKE_SOURCE_DIR}/res/Autumn8.manifest)
  if(EXISTS ${CMAKE_SOURCE_DIR}/res/Autumn8.ico)
    list(APPEND WINDOWS_RESOURCES ${CMAKE_SOURCE_DIR}/res/Autumn8.ico)
  else()
    set_source_files_properties(${CMAKE_SOURCE_DIR}/res/Autumn8.rc
      PROPERTIES COMPILE_DEFINITIONS ICON_MISSING
    )
  endif()
  set_source_files_properties(${CMAKE_SOURCE_DIR}/res/Autumn8.rc
    PROPERTIES OBJECT_DEPENDS "${WINDOWS_RESOURCES}"
  )
elseif(${A8_TARGET} STREQUAL "macOS")
  target_sources(guidriver PRIVATE platform/macos/macmain.mm)
endif()
