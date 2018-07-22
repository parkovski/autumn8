project(Autumn8Midi)

if(A8_TARGET STREQUAL "Windows")
  set(SOURCES uwp.cpp winmm.cpp)
else()
  message(FATAL_ERROR "MIDI not implemented for platform ${A8_TARGET}")
endif()

add_library(a8midi STATIC ${SOURCES})
