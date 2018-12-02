if(NOT bgfx_FOUND)
  message(FATAL_ERROR "This version of NanoVG relies on bgfx; import that first.")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/contrib/nanovg/src/nanovg.c")
  set(NanoVG_SOURCES
    "${PROJECT_SOURCE_DIR}/contrib/nanovg/src/nanovg.c"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx/nanovg_bgfx.cpp"
  )
  set(NanoVG_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/contrib/nanovg/src"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx"
  )
  add_library(NanoVG INTERFACE IMPORTED)
  target_sources(NanoVG INTERFACE ${NanoVG_SOURCES})
  target_include_directories(NanoVG INTERFACE
    ${NanoVG_INCLUDE_DIRS}
    ${bgfx_INCLUDE_DIRS}
  )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NanoVG
  REQUIRED_VARS NanoVG_SOURCES NanoVG_INCLUDE_DIRS
)
