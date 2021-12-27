project(Autumn8Utility)

set(SOURCES
  log.cpp
  mime.types.cpp
  router.cpp
  utfconv.cpp
)

add_library(a8util STATIC ${SOURCES})
target_link_libraries(a8util PUBLIC
  spdlog::spdlog
  fmt::fmt
  uriparser::uriparser
)
