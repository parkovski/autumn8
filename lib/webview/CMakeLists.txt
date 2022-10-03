project(Autumn8WebView)

set(SOURCES webview.cpp)

set(SOURCES_Win32 win32.cpp)
set(SOURCES_Gtk gtk.cpp)

add_library(a8webview STATIC ${SOURCES} ${SOURCES_${A8_GFXBACKEND}})
target_compile_definitions(a8webview PRIVATE A8_MODULE_NAME="a8-webview")
target_link_libraries(a8webview PUBLIC a8window)

if(A8_GFXBACKEND STREQUAL "Win32")
  target_include_directories(a8webview PRIVATE
    "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/include")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(a8webview PUBLIC
      "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x64")
  else()
    target_link_directories(a8webview PUBLIC
      "${CMAKE_SOURCE_DIR}/contrib/webview2/build/native/x86")
  endif()

  target_link_libraries(a8webview PUBLIC WebView2LoaderStatic)
  target_link_libraries(a8webview PRIVATE WIL::WIL)
elseif(A8_GFXBACKEND STREQUAL Gtk)
  target_link_libraries(a8webview PUBLIC PkgConfig::WebKitGTK)
endif()
