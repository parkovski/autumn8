project(Autumn8VM)

set(SOURCES
  ibuf.cpp
  scope.cpp
  stack.cpp
  string_map.cpp
  symbol_table.cpp
  vm.cpp
)

add_library(a8vm STATIC ${SOURCES})