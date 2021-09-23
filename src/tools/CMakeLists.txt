project(Autumn8Tools)

if(A8_HAS_GUI)
  add_executable(minigfx WIN32 minigfx.cpp)
  target_link_libraries(minigfx PRIVATE a8winmain a8gfx uWebSockets)
endif()

if(WIN32)
  add_executable(midictl midictl.cpp)
  target_link_libraries(midictl a8main a8midi)
endif()

if(fpsdk_FOUND)
  add_library(flsaw MODULE flsaw.cpp)
  a8_fl_plugin(flsaw A8Saw)
endif()
