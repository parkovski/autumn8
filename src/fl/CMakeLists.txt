project(Autumn8FL)

# set(FLLIB_SOURCES flmain.cpp)
set(FLPLUG_SOURCES plug/main.cpp)

# add_library(a8fl SHARED ${SOURCES})
# target_link_libraries(a8fl a8gfx)

add_library(flplugin SHARED ${SOURCES})
set_target_properties(flplugin PROPERTIES OUTPUT_NAME Autumn8FL)
target_link_libraries(flplugin a8gfx fpsdk)
