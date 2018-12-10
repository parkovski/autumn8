find_path(ffts_INCLUDE_DIR ffts/ffts.h)
find_library(ffts_LIBRARY NAMES ffts_static ffts_staticd)

add_library(ffts INTERFACE IMPORTED)
target_include_directories(ffts INTERFACE ${ffts_INCLUDE_DIR})
target_link_libraries(ffts INTERFACE ${ffts_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ffts
  REQUIRED_VARS ffts_INCLUDE_DIR ffts_LIBRARY
)
