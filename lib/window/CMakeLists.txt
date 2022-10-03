project(Autumn8Window)

set(SOURCES
  ostream_print.cpp
  view.cpp
)

set(SOURCES_Win32
  win32-init.cpp
  win32-msgloop.cpp
  win32-window.cpp
  win32-wndproc.cpp
)
set(SOURCES_Gtk
  gtk-init.cpp
  gtk-msgloop.cpp
  gtk-window.cpp
)

add_library(a8window STATIC ${SOURCES} ${SOURCES_${A8_GFXBACKEND}})
target_compile_definitions(a8window PRIVATE A8_MODULE_NAME="a8-window")
target_compile_definitions(a8window PUBLIC A8_HAS_GUI)
target_link_libraries(a8window PUBLIC a8util concurrentqueue)
target_include_directories(a8window PUBLIC ${Boost_INCLUDE_DIRS})

if(A8_GFXBACKEND STREQUAL "Win32")
  target_link_libraries(a8window PRIVATE WIL::WIL)
elseif(A8_GFXBACKEND STREQUAL "Gtk")
  target_link_libraries(a8window PUBLIC PkgConfig::GTK)
  # target_include_directories(a8window PRIVATE ${GTK_INCLUDE_DIRS})
  # target_compile_options(a8window PRIVATE ${GTK_CFLAGS_OTHER})
  # target_link_directories(a8window PUBLIC ${GTK_LIBRARY_DIRS})
  # target_link_libraries(a8window PUBLIC ${GTK_LIBRARIES})
endif()
