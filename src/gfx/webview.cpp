#include "a8/gfx/views/webview.h"

#include <boost/algorithm/string.hpp>

using namespace a8::gfx;

const char *ResponseStateException::kindToString(Kind kind) noexcept {
  switch (kind) {
    case DuplicateStatus:
      return "status called twice";
    case DuplicateContentType:
      return "contentType called twice";
    case DuplicateContentLength:
      return "contentLength called twice";
    case HeaderAfterContent:
      return "Header not allowed after content started";
    case WroteNotEnough:
      return "Closed before declared Content-Length written";
    case WroteTooMuch:
      return "Wrote more than declared Content-Length";
    case WriteAfterClose:
      return "Write called after response closed";
    case DuplicateEnd:
      return "End called twice";
    default:
      return "Unknown";
  }
}

static size_t streamWriteSize(std::streambuf &buf) {
  return buf.pubseekoff(0, std::ios::cur, std::ios::out);
}

WebViewResponse::WebViewResponse(void *platformData)
  : WebViewResponseStreamBase{},
    std::ostream{&this->_buffer},
    _platformData{platformData}
{}

WebViewResponse::~WebViewResponse() {
  endWrite();
}

WebViewResponse &WebViewResponse::status(http_status_t status) {
  if (_state >= WroteStatus) {
    throw ResponseStateException{ResponseStateException::DuplicateStatus};
  }
  _state = WroteStatus;

  _status = status.first;
  _reason = status.second;
  return *this;
}

WebViewResponse &WebViewResponse::contentType(std::string_view type) {
  if (_contentType.size()) {
    throw ResponseStateException{
      ResponseStateException::DuplicateContentType
    };
  }
  _contentType = type;
  return *this;
}

WebViewResponse &WebViewResponse::contentLength(size_t length) {
  if (_contentLength != ContentLengthUnknown) {
    throw ResponseStateException{
      ResponseStateException::DuplicateContentLength
    };
  }
  if (_state >= WroteContentPartial) {
    throw ResponseStateException{ResponseStateException::HeaderAfterContent};
  }
  _contentLength = length;
  return *this;
}

WebViewResponse &
WebViewResponse::header(std::string_view key, std::string_view value) {
  if (_state >= WroteContentPartial) {
    throw ResponseStateException{ResponseStateException::HeaderAfterContent};
  }

  if (boost::iequals(key, "content-type") ||
      boost::iequals(key, "content-length")) {
    throw std::domain_error{"Use contentType/contentLength methods"};
  } else {
    _headers.append(key).append(": ").append(value).append("\r\n");
  }
  return *this;
}

WebViewResponse &WebViewResponse::write(std::string_view data) {
  beginWrite();
  if (_state >= WroteContentFull) {
    throw ResponseStateException{ResponseStateException::WriteAfterClose};
  }

  const size_t contentLengthRemaining = streamWriteSize(this->_buffer);
  if (_contentLength != ContentLengthUnknown) {
    if (data.size() > contentLengthRemaining) {
      *this << data.substr(0, contentLengthRemaining);
      throw ResponseStateException{ResponseStateException::WroteTooMuch};
    } else if (data.size() == contentLengthRemaining) {
      _state = WroteContentFull;
    } else {
      _state = WroteContentPartial;
    }
  } else {
    _state = WroteContentPartial;
  }

  *this << data;
  return *this;
}

void WebViewResponse::end(std::string_view data) {
  if (data.length()) {
    write(data);
  }

  if (_state >= WroteAllClosed) {
    throw ResponseStateException{ResponseStateException::DuplicateEnd};
  }
  if (_contentLength != ContentLengthUnknown &&
      streamWriteSize(this->_buffer) < _contentLength) {
    throw ResponseStateException{ResponseStateException::WroteNotEnough};
  }

  endWrite();
}

void WebViewResponse::beginWrite() noexcept {
  switch (_state) {
    case WroteNone:
      Log::trace("WebViewResponse - default status 200 OK");
      _status = 200;
      _reason = http::HTTP_200_OK.second;
      [[fallthrough]];
    case WroteStatus:
      if (_contentType.empty()) {
        Log::trace("WebViewResponse - default content type text/plain");
        _contentType = "text/plain";
      }
      [[fallthrough]];
    default:
      break;
  }
}

void WebViewResponse::endWrite() noexcept {
  switch (_state) {
    case WroteNone:
      Log::trace("WebViewResponse - default status 500 Internal Server Error");
      _status = 500;
      _reason = http::HTTP_500_Internal_Server_Error.second;
      [[fallthrough]];
    case WroteStatus:
      Log::warn("WebViewResponse - no content");
      _contentType = "text/plain";
      _contentLength = 0;
      [[fallthrough]];
    case WroteContentPartial: {
      size_t writeSize = streamWriteSize(this->_buffer);
      if (_contentLength != ContentLengthUnknown &&
          writeSize < _contentLength) {
        Log::error("WebViewResponse closed but {} declared bytes not written",
                   _contentLength - writeSize);
        for (size_t i = 0; i < _contentLength - writeSize; ++i) {
          *this << '\0';
        }
      }
      [[fallthrough]];
    }
    case WroteContentFull:
      // Flush stream.
      finish();
      [[fallthrough]];
    case WroteAllClosed:
      break;
  }
}

bool WebView::callUserCallback(const WebViewRequest &req, WebViewResponse &res)
                               noexcept {
  if (!_userCallback) {
    return false;
  }
  try {
    _userCallback(req, res);
    return true;
  } catch (std::exception &e) {
    Log::error("WebView request for {} threw exception: {}",
               req.uri().path().str(), e.what());
    return false;
  }
}

namespace {
  struct RouteSeg {
    std::string_view route;
    size_t start = 0, end = 0;
    int state = 0;

    RouteSeg(std::string_view route) noexcept
      : route{route}
    {
      if (route[0] == '/') {
        route = route.substr(1);
      }
    }

    explicit operator bool() const noexcept {
      return end != std::string_view::npos;
    }

    std::string_view operator()() noexcept {
      end = route.find('/', start + 1);
      auto seg = route.substr(start, end - start);
      start = end + 1;
      return seg;
    }
  };
}
