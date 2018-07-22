find_path(sol_INCLUDE_DIR sol.hpp)

if(sol_INCLUDE_DIR)
  add_library(sol INTERFACE IMPORTED)
  target_include_directories(sol ${Sol_INCLUDE_DIR})

  file(READ "${sol_INCLUDE_DIR}/sol_forward.hpp" sol_forward_hpp)
  string(REGEX MATCH "generated with sol v([0-9]+)\.([0-9]+)\.([0-9]+) " _ sol_forward_hpp)
  set(sol_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(sol_VERSION_MINOR ${CMAKE_MATCH_2})
  set(sol_VERSION_PATCH ${CMAKE_MATCH_3})
  set(sol_VERSION "${sol_VERSION_MAJOR}.${sol_VERSION_MINOR}.${sol_VERSION_PATCH}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sol
  REQUIRED_VARS sol_INCLUDE_DIR
  VERSION_VAR sol_VERSION
)
