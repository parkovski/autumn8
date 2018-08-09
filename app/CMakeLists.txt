project(Autumn8App)

set(SOURCES
)

add_executable(Autumn8 WIN32 MACOSX_BUNDLE ${SOURCES})
target_link_libraries(Autumn8
  a8winmain
  a8gfx
  bgfx
  concurrentqueue
)

if(${A8_TARGET} STREQUAL "Windows")
  target_sources(Autumn8 PRIVATE
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
endif()
