#include "a8/webview/webview.h"

#include <boost/algorithm/string.hpp>

using namespace a8::webview;

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
{
  std::boolalpha(*this);
}

WebViewResponse::~WebViewResponse() {
  finish();
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
  // Don't need to check state because beginWrite fills in a default
  // content type if needed - will hit the above check.
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

void WebViewResponse::beginWrite() noexcept {
  switch (_state) {
    case WroteNone:
      A8_DEBUG("WebViewResponse - default status 200 OK");
      _status = 200;
      _reason = http::HTTP_200_OK.second;
      [[fallthrough]];
    case WroteStatus:
      if (_contentType.empty()) {
        A8_DEBUG("WebViewResponse - default content type text/plain");
        _contentType = "text/plain";
      }
      _state = WroteContentPartial;
      break;
    default:
      A8_ERROR("WebViewResponse::beginWrite called twice");
      break;
  }
}

void WebViewResponse::finish() noexcept {
  switch (_state) {
    case WroteNone:
      A8_DEBUG("WebViewResponse - default status 204 No Content");
      _status = 204;
      _reason = http::HTTP_204_No_Content.second;
      [[fallthrough]];
    case WroteStatus:
      if (_status == 200) {
        A8_WARN("WebViewResponse - status is 200 OK but no content given."
                " Recommend 204 No Content.");
      }
      _contentLength = 0;
      [[fallthrough]];
    case WroteContentPartial: {
      size_t writeSize = streamWriteSize(this->_buffer);
      if (_contentLength != ContentLengthUnknown &&
          writeSize < _contentLength) {
        A8_ERROR("WebViewResponse closed but {} declared bytes not written",
                 _contentLength - writeSize);
        for (size_t i = 0; i < _contentLength - writeSize; ++i) {
          *this << '\0';
        }
      }
      [[fallthrough]];
    }
    case WroteContentFull:
      // Flush stream.
      platformFinish();
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
    A8_ERROR("WebView request for {} threw exception: {}",
             req.uri().path().str(), e.what());
    return false;
  }
}
