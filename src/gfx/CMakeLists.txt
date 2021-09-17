project(Autumn8Graphics)

set(SOURCES
  common.cpp
  #nvgrenderer.cpp
  view.cpp
  window.cpp
)

set(SOURCES_Win32
  #backend/Win32/window.cpp
  #backend/Win32/wndproc.cpp
  backend/Win32/init.cpp
  backend/Win32/messageloop.cpp
  backend/Win32/webview.cpp
)

add_library(a8gfx STATIC ${SOURCES} ${SOURCES_${A8_GFXBACKEND}})
target_link_libraries(a8gfx PUBLIC
  a8platform
  a8util
  #bgfx::bgfx
  #NanoVG_bgfx
  concurrentqueue
)
target_include_directories(a8gfx PUBLIC ${Boost_INCLUDE_DIRS})

if(${A8_GFXBACKEND} STREQUAL "Win32")
  find_package(wil REQUIRED)

  target_include_directories(a8gfx PRIVATE ../../contrib/webview2/build/native/include)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(a8gfx PUBLIC "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x64")
  else()
    target_link_directories(a8gfx PUBLIC "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x86")
  endif()

  target_link_libraries(a8gfx PUBLIC WebView2LoaderStatic WIL::WIL)
endif()
