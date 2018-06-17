project(Autumn8Core)

set(SOURCES core.midi.uwp.cpp core.midi.winmm.cpp)

add_library(core SHARED ${SOURCES})
set_target_properties(core PROPERTIES OUTPUT_NAME a8core)
target_link_libraries(core cpprestsdk::cpprest)

add_executable(driver main.cpp)
set_target_properties(driver PROPERTIES OUTPUT_NAME a8)
target_link_libraries(driver core)
