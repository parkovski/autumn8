find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(SndFile REQUIRED) # SndFile::sndfile
find_package(Boost REQUIRED) # signals2

if(WIN32)
  set(uriparser_COMPONENTS char wchar_t)
else()
  set(uriparser_COMPONENTS char)
endif()
find_package(uriparser 0.9.2 REQUIRED COMPONENTS ${uriparser_COMPONENTS})

find_package(uWebSockets)

if(A8_BUILD_GUI AND A8_FLSTUDIO_PLATFORM)
  find_package(fpsdk)
endif()

if(A8_BUILD_TESTS)
  find_package(Catch2 3.0 REQUIRED)
endif()

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  "${PROJECT_SOURCE_DIR}/contrib/moodycamel/include"
)