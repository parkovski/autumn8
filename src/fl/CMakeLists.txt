project(Autumn8FL)

set(SOURCES flmain.cpp)

add_library(flplugin SHARED ${SOURCES})
set_target_properties(flplugin PROPERTIES OUTPUT_NAME Autumn8FL)
target_link_libraries(flplugin a8gfx fpsdk)
