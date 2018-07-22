project(Autumn8Gfx)

set(SOURCES
  nvgrenderer.cpp
  window.cpp
)

add_library(a8gfx ${SOURCES})
target_link_libraries(a8gfx
  bgfx
  fmt::fmt-header-only
  NanoVG
  concurrentqueue
)

if(${A8_TARGET} STREQUAL "Windows")
  target_sources(a8gfx PRIVATE
    platform/windows/messageloop.cpp
  )
endif()
