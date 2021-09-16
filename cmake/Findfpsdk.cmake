if(EXISTS "${PROJECT_SOURCE_DIR}/contrib/fpsdk/fp_cplug.h")
  file(GLOB fpsdk_SOURCES ${PROJECT_SOURCE_DIR}/contrib/fpsdk *.cpp)
  add_library(fpsdk INTERFACE IMPORTED)
  target_sources(fpsdk INTERFACE ${fpsdk_SOURCES})
  set(fpsdk_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/contrib/fpsdk")
  target_include_directories(fpsdk INTERFACE "${fpsdk_INCLUDE_DIR}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(fpsdk
  REQUIRED_VARS fpsdk_SOURCES fpsdk_INCLUDE_DIR
)
