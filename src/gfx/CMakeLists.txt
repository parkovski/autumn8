project(Autumn8Graphics)

set(SOURCES
  common.cpp
  #nvgrenderer.cpp
  #view.cpp
  window.cpp
)

add_library(a8gfx STATIC ${SOURCES})
target_link_libraries(a8gfx PUBLIC
  a8util
  #bgfx::bgfx
  #NanoVG_bgfx
  concurrentqueue
)
target_include_directories(a8gfx PUBLIC ${Boost_INCLUDE_DIRS})

if(${A8_GFXBACKEND} STREQUAL "Win32")
  target_sources(a8gfx PRIVATE
    backend/Win32/messageloop.cpp
  )
endif()
