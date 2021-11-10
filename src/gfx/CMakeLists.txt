project(Autumn8Graphics)

set(SOURCES
  ostream_print.cpp
  view.cpp
)

set(SOURCES_Win32
  backend/Win32/init.cpp
  backend/Win32/mainwndproc.cpp
  backend/Win32/messageloop.cpp
  backend/Win32/webview.cpp
  backend/Win32/window.cpp
)

set(SOURCES_Gtk
  backend/Gtk/init.cpp
  backend/Gtk/messageloop.cpp
  backend/Gtk/webview.cpp
  backend/Gtk/window.cpp
)

add_library(a8gfx STATIC ${SOURCES} ${SOURCES_${A8_GFXBACKEND}})
target_compile_definitions(a8gfx PUBLIC A8_HAS_GUI)
target_link_libraries(a8gfx PUBLIC
  a8util
  concurrentqueue
)

if(A8_HAS_BGFX)
  target_link_libraries(a8gfx PRIVATE
    bgfx::bgfx
    NanoVG_bgfx
  )
endif()

target_include_directories(a8gfx PUBLIC ${Boost_INCLUDE_DIRS})

if(A8_GFXBACKEND STREQUAL "Win32")
  find_package(wil REQUIRED)

  target_include_directories(a8gfx PRIVATE ../../contrib/webview2/build/native/include)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(a8gfx PUBLIC "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x64")
  else()
    target_link_directories(a8gfx PUBLIC "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x86")
  endif()

  target_link_libraries(a8gfx PUBLIC WebView2LoaderStatic)
  target_link_libraries(a8gfx PRIVATE WIL::WIL)
elseif(A8_GFXBACKEND STREQUAL "Gtk")
  find_package(PkgConfig REQUIRED)
  if(A8_GTK_VERSION STREQUAL 3)
    pkg_check_modules(GTK REQUIRED gtk+-3.0 webkit2gtk-4.0)
  elseif(A8_GTK_VERSION STREQUAL 4)
    pkg_check_modules(GTK REQUIRED gtk4 webkit2gtk-5.0)
  else()
    message(FATAL_ERROR "A8_GTK_VERSION ${A8_GTK_VERSION} should be 3 or 4")
  endif()

  target_include_directories(a8gfx PRIVATE ${GTK_INCLUDE_DIRS})
  target_compile_options(a8gfx PRIVATE ${GTK_CFLAGS_OTHER})
  target_link_directories(a8gfx PUBLIC ${GTK_LIBRARY_DIRS})
  target_link_libraries(a8gfx PUBLIC ${GTK_LIBRARIES})
endif()
