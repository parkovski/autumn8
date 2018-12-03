project(Autumn8Main)

add_library(a8main INTERFACE)
target_sources(a8main INTERFACE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

if(A8_TARGET STREQUAL "Windows")
  add_library(a8winmain INTERFACE)
  # for utf conversion
  target_link_libraries(a8winmain INTERFACE a8util)
  target_sources(a8winmain INTERFACE ${CMAKE_CURRENT_LIST_DIR}/winmain.cpp)
else()
  add_library(a8winmain ALIAS a8main)
endif()
