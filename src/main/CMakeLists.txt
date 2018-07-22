project(Autumn8Main)

add_library(a8main INTERFACE)
target_sources(a8main INTERFACE main.cpp)

if(A8_TARGET STREQUAL "Windows")
  add_library(a8winmain INTERFACE)
  target_sources(a8winmain INTERFACE winmain.cpp)
else()
  add_library(a8winmain ALIAS a8main)
endif()
