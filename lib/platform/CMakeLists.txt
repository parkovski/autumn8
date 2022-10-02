project(Autumn8Platform)

# add_library(a8platform OBJECT ${A8_PLATFORM}.cpp)
# target_compile_definitions(a8platform PRIVATE A8_MODULE_NAME="a8-main")
# Logger
# target_link_libraries(a8platform PUBLIC a8util)

add_library(a8main STATIC main.cpp ${A8_PLATFORM}.cpp)
target_compile_definitions(a8main PRIVATE A8_MODULE_NAME="a8-main")
target_link_libraries(a8main PUBLIC a8util)

if(A8_PLATFORM STREQUAL "Windows")
  target_link_options(a8main INTERFACE "/ENTRY:wmainCRTStartup")
endif()

if(NOT A8_BUILD_GUI)
  return()
endif()

if(A8_PLATFORM STREQUAL "Windows")
  add_library(a8gfxmain STATIC winmain.cpp Windows.cpp)
  target_link_options(a8gfxmain INTERFACE "/ENTRY:wWinMainCRTStartup")
elseif(A8_PLATFORM STREQUAL "Android")
  add_library(a8gfxmain STATIC
    androidmain.cpp
    Android.cpp
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
  )
  target_include_directories(a8gfxmain PRIVATE
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
  target_link_options(a8gfxmain INTERFACE "-u ANativeActivity_onCreate")
  #set_property(a8gfxmain APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
else()
  add_library(a8gfxmain STATIC main.cpp ${A8_PLATFORM}.cpp)
endif()

target_compile_definitions(a8gfxmain PRIVATE A8_MODULE_NAME="a8-main")
target_compile_definitions(a8gfxmain PUBLIC A8_HAS_GUI)
target_link_libraries(a8gfxmain PUBLIC a8util a8gfx)
