#include "a8/gfx/views/webview.h"

#include <boost/algorithm/string.hpp>

using namespace a8::gfx;

WebViewResponse &
WebViewResponse::header(std::string_view key, std::string_view value) {
  if (boost::iequals(key, "content-type")) {
    contentType(value);
  } else {
    _headers.append(key).append(": ").append(value).append("\r\n");
  }
  return *this;
}
