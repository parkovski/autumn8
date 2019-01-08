project(Autumn8Utility)

#add_library(sds INTERFACE IMPORTED)
#target_sources(sds INTERFACE ${CMAKE_SOURCE_DIR}/contrib/sds/sds.c)
#set_source_files_properties(${CMAKE_SOURCE_DIR}/contrib/sds/sds.c PROPERTIES LANGUAGE C)
#target_include_directories(sds INTERFACE ${CMAKE_SOURCE_DIR}/contrib/sds)

set(SOURCES utfconv.cpp)

add_library(a8util STATIC ${SOURCES})
