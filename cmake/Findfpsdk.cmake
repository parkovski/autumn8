find_path(fpsdk_DIR source/c/fp_cplug.h HINTS "$ENV{fpsdk_DIR}")

if(fpsdk_DIR AND EXISTS "${fpsdk_DIR}/source/c/fp_cplug.cpp")
  file(GLOB fpsdk_SOURCES ${fpsdk_DIR}/source/c *.cpp)
  add_library(fpsdk INTERFACE IMPORTED)
  target_sources(fpsdk INTERFACE ${fpsdk_SOURCES})
  set(fpsdk_INCLUDE_DIR "${fpsdk_DIR}/source/c")
  target_include_directories(fpsdk INTERFACE "${fpsdk_INCLUDE_DIR}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(fpsdk
  REQUIRED_VARS fpsdk_SOURCES fpsdk_INCLUDE_DIR
)
