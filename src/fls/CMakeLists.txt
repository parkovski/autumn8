project(Autumn8FLStudio)

# set(FLSLIB_SOURCES flmain.cpp)
set(FLSPLUG_SOURCES plug/main.cpp)

# add_library(a8fls STATIC ${SOURCES})
# target_link_libraries(a8fls a8graphics)

add_library(a8flsplug SHARED ${SOURCES})
set_target_properties(a8flsplug PROPERTIES OUTPUT_NAME Autumn8.FLSPlugin)
target_link_libraries(a8flsplug a8gfx fpsdk)
