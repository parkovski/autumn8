project(Autumn8Midi)

set(SOURCES_Windows winmm.cpp)
set(SOURCES_Linux alsa.cpp)

if(NOT SOURCES_${A8_PLATFORM})
  message(FATAL_ERROR "MIDI not implemented for platform ${A8_PLATFORM}")
endif()

add_library(a8midi STATIC common.cpp ${SOURCES_${A8_PLATFORM}})
target_link_libraries(a8midi PUBLIC a8util)
