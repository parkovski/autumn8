set(SOURCES
  test.cpp
  paramconv.cpp
  router.cpp
  uri.cpp
  utf.cpp
  vm.cpp
)

add_executable(test ${SOURCES})

target_link_libraries(test
  Catch2::Catch2
  a8vm
  a8util
)

include(CTest)
include(Catch)
catch_discover_tests(test)
