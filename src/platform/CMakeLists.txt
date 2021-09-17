project(Autumn8Platform)

set(SOURCES_Windows
  Windows/platform.cpp
)

add_library(a8nogfx OBJECT nogfx.cpp)

add_library(a8platform STATIC ${SOURCES_${A8_PLATFORM}})
target_link_libraries(a8platform PUBLIC a8util)
