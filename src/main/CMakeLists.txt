project(Autumn8Main)

add_library(a8main STATIC main.cpp platform/${A8_PLATFORM}.cpp)
# Logger
target_link_libraries(a8main PUBLIC a8util)

if(A8_PLATFORM STREQUAL "Windows")
  target_link_options(a8main INTERFACE "/ENTRY:wmainCRTStartup")
endif()

if(NOT A8_BUILD_GUI)
  return()
endif()

if(A8_PLATFORM STREQUAL "Windows")
  add_library(a8gfxmain INTERFACE)
  add_library(a8gfxmain STATIC winmain.cpp platform/Windows.cpp)
  target_link_options(a8gfxmain INTERFACE "/ENTRY:wWinMainCRTStartup")
elseif(A8_PLATFORM STREQUAL "Android")
  add_library(a8gfxmain STATIC
    androidmain.cpp
    platform/Android.cpp
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
  )
  target_include_directories(a8gfxmain PRIVATE
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
  target_link_options(a8gfxmain INTERFACE "-u ANativeActivity_onCreate")
  #set_property(a8gfxmain APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
else()
  add_library(a8gfxmain STATIC main.cpp platform/${A8_PLATFORM}.cpp)
endif()

target_compile_definitions(a8gfxmain PUBLIC A8_HAS_GUI)
target_link_libraries(a8gfxmain PUBLIC a8util a8gfx)
