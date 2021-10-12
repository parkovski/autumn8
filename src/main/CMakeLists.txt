project(Autumn8Main)

add_library(a8main STATIC main.cpp)
# Logger
target_link_libraries(a8main PUBLIC a8util)

if(A8_PLATFORM STREQUAL "Windows")
  target_link_options(a8main INTERFACE "/ENTRY:wmainCRTStartup")

  add_library(a8sysmain STATIC winmain.cpp)
  # Logger / UTF conversion
  target_link_libraries(a8sysmain PUBLIC a8util)
  target_link_options(a8sysmain INTERFACE "/ENTRY:wWinMainCRTStartup")
elseif(A8_PLATFORM STREQUAL "Android")
  add_library(a8sysmain STATIC
    "${CMAKE_CURRENT_LIST_DIR}/androidmain.cpp"
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
  )
  target_include_directories(a8sysmain PRIVATE
    "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
  target_link_libraries(a8sysmain PUBLIC a8util)
  target_link_options(a8sysmain INTERFACE "-u ANativeActivity_onCreate")
  #set_property(a8sysmain APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
else()
  add_library(a8sysmain ALIAS a8main)
endif()
