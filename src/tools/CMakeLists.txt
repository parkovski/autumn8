project(Autumn8Tools)

if(A8_HAS_GUI)
  add_executable(minigfx WIN32 minigfx.cpp)
  target_link_libraries(minigfx PRIVATE a8winmain a8gfx)
endif()

if(WIN32)
  add_executable(midictl midictl.cpp)
  target_link_libraries(midictl a8main a8midi)
endif()
