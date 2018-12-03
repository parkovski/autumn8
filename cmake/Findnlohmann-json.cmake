find_path(nlohmann_json_INCLUDE_DIR nlohmann/json.hpp)

if(nlohmann_json_INCLUDE_DIR)
  add_library(nlohmann-json INTERFACE IMPORTED)
  target_include_directories(nlohmann-json INTERFACE ${nlohmann_json_INCLUDE_DIR})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nlohmann-json
  REQUIRED_VARS nlohmann_json_INCLUDE_DIR
  # VERSION_VAR nlohmann_json_VERSION
)
