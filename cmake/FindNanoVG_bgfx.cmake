if(NOT bgfx_FOUND)
  message(FATAL_ERROR "This version of NanoVG relies on bgfx; import that first.")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/contrib/nanovg/src/nanovg.c")
  set(NanoVG_bgfx_SOURCES
    "${PROJECT_SOURCE_DIR}/contrib/nanovg/src/nanovg.c"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx/nanovg_bgfx.cpp"
  )
  set(NanoVG_bgfx_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/contrib/nanovg/src"
    "${PROJECT_SOURCE_DIR}/contrib/nanovg_bgfx"
  )
  add_library(NanoVG_bgfx INTERFACE IMPORTED)
  target_sources(NanoVG_bgfx INTERFACE ${NanoVG_bgfx_SOURCES})
  if(MSVC)
    target_compile_definitions(NanoVG_bgfx INTERFACE -D_CRT_SECURE_NO_WARNINGS)
  endif()
  target_include_directories(NanoVG_bgfx INTERFACE
    ${NanoVG_bgfx_INCLUDE_DIRS}
    ${bgfx_INCLUDE_DIRS}
  )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NanoVG_bgfx
  REQUIRED_VARS NanoVG_bgfx_SOURCES NanoVG_bgfx_INCLUDE_DIRS
)
