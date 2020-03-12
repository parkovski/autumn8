find_path(uWebSockets_INCLUDE_DIR uwebsockets/WebSocket.h)
find_library(uSockets_LIBRARY NAMES uSockets)

if(NOT EXISTS "${uWebSockets_INCLUDE_DIR}/libusockets.h")
  message(FATAL_ERROR "Missing libusockets.h")
endif()

add_library(uWebSockets INTERFACE IMPORTED)
target_include_directories(uWebSockets INTERFACE ${uWebSockets_INCLUDE_DIR})
target_link_libraries(uWebSockets INTERFACE ${uSockets_LIBRARY})

find_package(ZLIB)
if(ZLIB_FOUND)
  target_link_libraries(uWebSockets INTERFACE ZLIB::ZLIB)
else()
  target_compile_definitions(uWebSockets -DUWS_NO_ZLIB)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(uWebSockets
  REQUIRED_VARS uWebSockets_INCLUDE_DIR uSockets_LIBRARY
)
