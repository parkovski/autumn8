find_path(uWebSockets_INCLUDE_DIRS uwebsockets/App.h)
find_library(uSockets_LIBRARY NAMES uSockets)

if(NOT EXISTS "${uWebSockets_INCLUDE_DIRS}/libusockets.h")
  message(FATAL_ERROR "Missing libusockets.h")
endif()

add_library(uWebSockets INTERFACE IMPORTED)

find_package(ZLIB QUIET)
if(ZLIB_FOUND)
  message(STATUS "uWebSockets using zlib")
  target_link_libraries(uWebSockets INTERFACE ZLIB::ZLIB)
else()
  target_compile_definitions(uWebSockets INTERFACE -DUWS_NO_ZLIB)
endif()

find_path(LIBUV_INCLUDE_DIR uv.h)
find_library(LIBUV_LIBRARY NAMES libuv)
target_include_directories(uWebSockets INTERFACE
  ${uWebSockets_INCLUDE_DIRS} ${LIBUV_INCLUDE_DIR})
target_link_libraries(uWebSockets INTERFACE
  ${uSockets_LIBRARY} ${LIBUV_LIBRARY})

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
  message(STATUS "uWebSockets using OpenSSL")
  target_link_libraries(uWebSockets INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(uWebSockets REQUIRED_VARS
  uWebSockets_INCLUDE_DIRS uSockets_LIBRARY LIBUV_INCLUDE_DIR LIBUV_LIBRARY
)
