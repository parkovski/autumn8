project(Autumn8Gfx)

set(SOURCES gfxmain.cpp)

add_library(gfx ${SOURCES})
set_target_properties(gfx PROPERTIES OUTPUT_NAME a8gfx)
target_link_libraries(gfx bgfx fmt::fmt NanoVG Yoga concurrentqueue)

add_executable(guidriver WIN32 MACOSX_BUNDLE)
set_target_properties(guidriver PROPERTIES OUTPUT_NAME Autumn8)
target_link_libraries(guidriver gfx bgfx concurrentqueue)
if(WIN32)
  target_sources(guidriver PRIVATE winmain.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_sources(guidriver PRIVATE macmain.mm)
endif()
