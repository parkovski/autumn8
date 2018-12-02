find_path(Sol2_INCLUDE_DIR sol.hpp)

if(Sol2_INCLUDE_DIR)
  add_library(Sol2 INTERFACE IMPORTED)
  target_include_directories(Sol2 INTERFACE ${Sol2_INCLUDE_DIR})

  # file(READ "${Sol2_INCLUDE_DIR}/sol.hpp" Sol2_hpp)
  # string(REGEX MATCH "generated with sol v([0-9]+)\.([0-9]+)\.([0-9]+) " _ Sol2_hpp)
  # set(Sol2_VERSION_MAJOR ${CMAKE_MATCH_1})
  # set(Sol2_VERSION_MINOR ${CMAKE_MATCH_2})
  # set(Sol2_VERSION_PATCH ${CMAKE_MATCH_3})
  # set(Sol2_VERSION "${Sol2_VERSION_MAJOR}.${Sol2_VERSION_MINOR}.${Sol2_VERSION_PATCH}")
  # unset(Sol2_hpp)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sol
  REQUIRED_VARS Sol2_INCLUDE_DIR
  # VERSION_VAR Sol2_VERSION
)
