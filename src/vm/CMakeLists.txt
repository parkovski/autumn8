project(Autumn8VM)

set(SOURCES
  data.cpp
  execution_context.cpp
  ibuf.cpp
  string_map.cpp
  symbol.cpp
  symbol_table.cpp
)

add_library(a8vm STATIC ${SOURCES})