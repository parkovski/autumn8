find_package(Catch2 3.0 REQUIRED)

set(SOURCES
  test.cpp
  paramconv.cpp
  utf.cpp
)

add_executable(test ${SOURCES})

target_link_libraries(test
  Catch2::Catch2
  a8util
)

include(CTest)
include(Catch)
catch_discover_tests(test)
