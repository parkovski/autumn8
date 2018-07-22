find_path(plog_INCLUDE_DIR plog/Log.h)

add_library(plog INTERFACE IMPORTED)
target_include_directories(plog INTERFACE ${plog_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(plog
  REQUIRED_VARS plog_INCLUDE_DIR
)
