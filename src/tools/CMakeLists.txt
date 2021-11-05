project(Autumn8Tools)

if(A8_HAS_GUI)
  add_executable(minigfx WIN32 minigfx.cpp)
  target_link_libraries(minigfx PRIVATE a8sysmain a8gfx)

  if(uWebSockets_FOUND)
    add_executable(uws-server WIN32 uws-server.cpp)
    target_link_libraries(uws-server PRIVATE a8sysmain a8gfx uWebSockets)
  endif()
endif()

add_executable(midictl midictl.cpp)
target_link_libraries(midictl a8main a8midi)

if(fpsdk_FOUND)
  add_library(flsaw MODULE flsaw.cpp)
  a8_fl_plugin(flsaw A8Saw)
endif()
