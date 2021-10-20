find_path(uWebSockets_INCLUDE_DIR uwebsockets/App.h)
find_path(uSockets_INCLUDE_DIR libusockets.h HINTS ${uWebSockets_INCLUDE_DIR})
find_library(uSockets_LIBRARY NAMES uSockets)

set(uWebSockets_INCLUDE_DIRS ${uWebSockets_INCLUDE_DIR} ${uSockets_INCLUDE_DIR})

add_library(uWebSockets INTERFACE IMPORTED)
target_include_directories(uWebSockets INTERFACE ${uWebSockets_INCLUDE_DIRS})
target_link_libraries(uWebSockets INTERFACE ${uSockets_LIBRARY})

if(NOT UWS_NO_ZLIB)
  find_package(ZLIB QUIET)
  if(ZLIB_FOUND)
    message(STATUS "uWebSockets using zlib")
    target_link_libraries(uWebSockets INTERFACE ZLIB::ZLIB)
  else()
    target_compile_definitions(uWebSockets INTERFACE -DUWS_NO_ZLIB)
  endif()
endif()

if(WIN32 OR UWS_USE_LIBUV)
  find_path(LIBUV_INCLUDE_DIR uv.h REQUIRED)
  find_library(LIBUV_LIBRARY NAMES libuv REQUIRED)
  target_include_directories(uWebSockets INTERFACE ${LIBUV_INCLUDE_DIR})
  target_link_libraries(uWebSockets INTERFACE ${LIBUV_LIBRARY})
endif()

if(NOT UWS_NO_OPENSSL)
  find_package(OpenSSL QUIET)
  if(OpenSSL_FOUND)
    message(STATUS "uWebSockets using OpenSSL")
    target_link_libraries(uWebSockets INTERFACE OpenSSL::SSL OpenSSL::Crypto)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(uWebSockets REQUIRED_VARS
  uWebSockets_INCLUDE_DIRS uSockets_LIBRARY
)
