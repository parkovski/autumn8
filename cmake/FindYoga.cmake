find_path(Yoga_INCLUDE_DIR yoga/Yoga.h)
find_library(Yoga_LIBRARIES yogacore)

add_library(Yoga INTERFACE IMPORTED)
target_include_directories(Yoga INTERFACE ${Yoga_INCLUDE_DIR})
target_link_libraries(Yoga INTERFACE ${Yoga_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yoga
  REQUIRED_VARS Yoga_INCLUDE_DIR Yoga_LIBRARIES
)
