find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(SndFile REQUIRED) # SndFile::sndfile
find_package(Boost REQUIRED) # signals2
find_package(nlohmann_json 3.10.5 REQUIRED)

if(WIN32)
  set(uriparser_COMPONENTS char wchar_t)
else()
  set(uriparser_COMPONENTS char)
endif()
find_package(uriparser 0.9.2 REQUIRED COMPONENTS ${uriparser_COMPONENTS})

find_package(uWebSockets)

if(A8_BUILD_GUI)
  if(A8_GFXBACKEND STREQUAL "Win32")
    find_package(wil REQUIRED)
  elseif(A8_GFXBACKEND STREQUAL "Gtk")
    find_package(PkgConfig REQUIRED)
    if(A8_GTK_VERSION STREQUAL 3)
      pkg_check_modules(GTK REQUIRED IMPORTED_TARGET GLOBAL gtk+-3.0)
      pkg_check_modules(WebKitGTK IMPORTED_TARGET GLOBAL webkit2gtk-4.1)
    elseif(A8_GTK_VERSION STREQUAL 4)
      pkg_check_modules(GTK REQUIRED IMPORTED_TARGET GLOBAL gtk4)
      pkg_check_modules(WebKitGTK IMPORTED_TARGET GLOBAL webkit2gtk-5.0)
    else()
      message(FATAL_ERROR "A8_GTK_VERSION ${A8_GTK_VERSION} should be 3 or 4")
    endif()
  endif()

  if(A8_FLSTUDIO_PLATFORM)
    find_package(fpsdk)
  endif()
endif()

if(A8_BUILD_TESTS)
  find_package(Catch2 3.0 REQUIRED)
endif()

add_library(concurrentqueue INTERFACE IMPORTED)
target_include_directories(concurrentqueue INTERFACE
  "${PROJECT_SOURCE_DIR}/contrib/moodycamel/include"
)