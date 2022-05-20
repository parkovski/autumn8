project(Autumn8Utility)

set(SOURCES
  log.cpp
  mime.types.cpp
  router.cpp
  utfconv.cpp
)

add_library(a8util STATIC ${SOURCES})
target_compile_definitions(a8util PRIVATE A8_MODULE_NAME="a8-util")
target_link_libraries(a8util PUBLIC
  spdlog::spdlog
  fmt::fmt
  uriparser::uriparser
)
