set(SOURCES
  test.cpp
  paramconv.cpp
  router.cpp
  uri.cpp
  utf.cpp
)

add_executable(test ${SOURCES})
target_compile_definitions(test PRIVATE A8_MODULE_NAME="a8-test")

target_link_libraries(test
  Catch2::Catch2
  a8util
)

include(CTest)
include(Catch)
catch_discover_tests(test)
