if(NOT bgfx_INCLUDE_DIRS)
  message(FATAL_ERROR "This version of NanoVG relies on bgfx; import that first.")
endif()

find_path(NanoVG_DIR src/nanovg.c HINTS "$ENV{NanoVG_DIR}")

if(NanoVG_DIR AND EXISTS "${NanoVG_DIR}/src/nanovg.c")
  set(NanoVG_SOURCES
    "${NanoVG_DIR}/src/nanovg.c"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx/nanovg_bgfx.cpp"
  )
  set(NanoVG_INCLUDE_DIRS
    "${NanoVG_DIR}/src"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx"
  )
  add_library(NanoVG STATIC ${NanoVG_SOURCES})
  target_include_directories(NanoVG PUBLIC ${NanoVG_INCLUDE_DIRS} ${bgfx_INCLUDE_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NanoVG REQUIRED_VARS NanoVG_SOURCES NanoVG_INCLUDE_DIRS)
