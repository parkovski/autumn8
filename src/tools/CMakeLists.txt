project(Autumn8Tools)

add_executable(minigfx WIN32 minigfx.cpp)
target_link_libraries(minigfx PRIVATE a8winmain a8gfx)

# add_executable(fftexplorer fftexplorer.cpp)
# target_link_libraries(fftexplorer a8main ffts)

add_executable(server server.cpp)
target_link_libraries(server a8main uWebSockets)

add_executable(auconvert auconvert.cpp)
target_link_libraries(auconvert a8main ni::audiostream ni::pcm bgfx::bimg)

add_executable(rna_to_wav rna_to_wav.cpp)
target_link_libraries(rna_to_wav a8main ni::audiostream ni::pcm)

add_executable(rna_to_wav2 rna_to_wav2.cpp)
target_link_libraries(rna_to_wav2 a8main ni::audiostream ni::pcm)

if(WIN32)
  add_executable(midictl midictl.cpp)
  target_link_libraries(midictl a8main a8midi)
endif()
