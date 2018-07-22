project(Autumn8Tools)

set(SOURCES)

add_executable(midilist midilist.cpp)
target_link_libraries(midilist a8midi)
