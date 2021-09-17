project(Autumn8FLPlugin)

set(SOURCES
  flplug.cpp
)

add_library(a8flp STATIC ${SOURCES})
target_link_libraries(a8flp PRIVATE fpsdk)
target_link_libraries(a8flp PUBLIC
  a8gfx
)

# Temporary:
target_include_directories(a8flp PUBLIC ${fpsdk_INCLUDE_DIR})

function(a8_fl_plugin TARGET_NAME OUTPUT_NAME)
  target_link_libraries(${TARGET_NAME} PRIVATE a8flp)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_NAME "${OUTPUT_NAME}_x64")
  else()
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_NAME "${OUTPUT_NAME}")
  endif()
endfunction()
