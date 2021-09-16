project(Autumn8FLPlugin)

set(SOURCES
  flplug.cpp
  ../../contrib/fpsdk/fp_plugclass.cpp
  ../../contrib/fpsdk/fp_cplug.cpp
  ../../contrib/fpsdk/fp_extra.cpp
)

add_library(a8flp MODULE ${SOURCES})
target_link_libraries(a8flp PRIVATE
  fpsdk
  a8gfx
)

function(a8_fl_plugin TARGET_NAME OUTPUT_NAME)
  #target_link_libraries(${TARGET_NAME} PRIVATE a8flp)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_NAME "${OUTPUT_NAME}_x64")
  else()
    set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_NAME "${OUTPUT_NAME}")
  endif()
endfunction()

a8_fl_plugin(a8flp A8Saw)
