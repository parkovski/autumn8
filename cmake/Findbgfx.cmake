find_path(bgfx_DIR include/bgfx/bgfx.h)
find_path(bx_DIR include/bx/bx.h)
find_path(bimg_DIR include/bimg/bimg.h)

if(bgfx_DIR AND bx_DIR AND bimg_DIR)
  set(bgfx_LIB_NAME debug bgfxd optimized bgfx)
  set(bx_LIB_NAME debug bxd optimized bx)
  set(bimg_LIB_NAME debug bimgd optimized bimg)

  find_library(bgfx_LIB NAMES ${bgfx_LIB_NAME})
  find_library(bx_LIB NAMES ${bx_LIB_NAME})
  find_library(bimg_LIB NAMES ${bimg_LIB_NAME})

  add_library(bgfx INTERFACE IMPORTED)
  set(bgfx_LIBRARIES ${bgfx_LIB} ${bx_LIB} ${bimg_LIB})
  set(bgfx_INCLUDE_DIRS ${bgfx_DIR}/include ${bx_DIR}/include ${bimg_DIR}/include)

  if(MSVC)
    list(APPEND bgfx_INCLUDE_DIRS ${bx_DIR}/include/compat/msvc)
  elseif(MINGW)
    list(APPEND bgfx_INCLUDE_DIRS ${bx_DIR}/include/compat/mingw)
  elseif(A8_PLATFORM STREQUAL "macOS")
    list(APPEND bgfx_INCLUDE_DIRS ${bx_DIR}/include/compat/osx)
  elseif(A8_PLATFORM STREQUAL "iOS")
    list(APPEND bgfx_INCLUDE_DIRS ${bx_DIR}/include/compat/ios)
  elseif(A8_PLATFORM STREQUAL "FreeBSD")
    list(APPEND bgfx_INCLUDE_DIRS ${bx_DIR}/include/compat/freebsd)
  endif()

  target_link_libraries(bgfx INTERFACE ${bgfx_LIBRARIES})
  target_include_directories(bgfx INTERFACE ${bgfx_INCLUDE_DIRS})
endif() # bgfx_DIR

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(bgfx REQUIRED_VARS bgfx_INCLUDE_DIRS bgfx_LIBRARIES)
