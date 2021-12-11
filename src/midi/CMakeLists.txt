project(Autumn8Midi)

set(SOURCES_Windows winmm.cpp)
# set(SOURCES_Windows winmm.cpp uwp.cpp)
set(SOURCES_macOS coremidi.cpp)
set(SOURCES_iOS coremidi.cpp)
set(SOURCES_Android amidi.cpp androidmediamidi.cpp)
set(SOURCES_Linux alsaseq.cpp alsaraw.cpp)
#set(SOURCES_Linux alsa.cpp alsaseq.cpp alsaraw.cpp)
set(SOURCES_Emscripten webmidi.cpp)

if(NOT SOURCES_${A8_PLATFORM})
  message(WARNING "MIDI not implemented for platform ${A8_PLATFORM}")
endif()

add_library(a8midi STATIC common.cpp null.cpp ${SOURCES_${A8_PLATFORM}})
#if(A8_HAS_PIPEWIRE)
#  target_sources(a8midi PRIVATE pipewire.cpp)
#endif()
#if(A8_HAS_JACK)
#  target_sources(a8midi PRIVATE jack.cpp)
#endif()
target_link_libraries(a8midi PUBLIC a8util)

# Boost::UUID
target_include_directories(a8midi PUBLIC ${Boost_INCLUDE_DIRS})

if(A8_PLATFORM STREQUAL Linux)
  find_package(ALSA REQUIRED)
  target_link_libraries(a8midi PUBLIC ALSA::ALSA)
endif()
