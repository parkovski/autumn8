project(Autumn8Tools)

add_executable(midilist midilist.cpp)
target_link_libraries(midilist a8midi)

add_executable(netcorehost netcorehost.cpp)
target_include_directories(netcorehost PRIVATE ${CMAKE_SOURCE_DIR}/contrib/dotnet/include)
#target_link_libraries(netcorehost)

add_executable(minigfx WIN32 minigfx.cpp)
target_link_libraries(minigfx PRIVATE a8winmain a8gfx)

add_executable(fftexplorer fftexplorer.cpp)
target_link_libraries(fftexplorer a8main ffts)

add_executable(server server.cpp)
target_link_libraries(server a8main uWebSockets)
