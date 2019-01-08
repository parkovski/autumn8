project(Autumn8Script)

set(SOURCES script.cpp document.cpp lexer.cpp parser.cpp exec.cpp)

add_library(a8script SHARED ${SOURCES})
target_compile_definitions(a8script PRIVATE -DBUILDING_DLL)
