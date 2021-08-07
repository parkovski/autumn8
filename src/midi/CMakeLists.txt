project(Autumn8Midi)

if(A8_PLATFORM STREQUAL "Windows")
  #set(PLATFORM_SOURCES uwp.cpp winmm.cpp)
  set(PLATFORM_SOURCES winmm.cpp)
else()
  message(FATAL_ERROR "MIDI not implemented for platform ${A8_PLATFORM}")
endif()

add_library(a8midi STATIC common.cpp ${PLATFORM_SOURCES})
target_link_libraries(a8midi PUBLIC a8util)
