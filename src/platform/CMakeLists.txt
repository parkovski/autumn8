project(Autumn8Platform)

add_library(a8nogfx OBJECT nogfx.cpp)

add_library(a8platform STATIC ${A8_PLATFORM}/platform.cpp)
target_link_libraries(a8platform PUBLIC a8util)
