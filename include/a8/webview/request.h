#ifndef A8_WEBVIEW_REQUEST_H_
#define A8_WEBVIEW_REQUEST_H_

#include "a8/util/uri.h"
#include "a8/util/router.h"

namespace a8::wv {

class WebView;

struct WebViewRequest {
private:
  WebView *_webview;
  Uri _uri;

public:
  WebViewRequest(WebView *webview, Uri &&uri)
    : _webview{webview}, _uri{std::move(uri)}
  {}

  WebView *webview() const noexcept { return _webview; }
  const Uri &uri() const noexcept { return _uri; }
};

struct WebViewRouterRequest : RouterBase::RouteParams {
private:
  const WebViewRequest *_wvRequest;

public:
  explicit WebViewRouterRequest(const WebViewRequest *wvRequest)
    : _wvRequest{wvRequest}
  {}

  WebViewRouterRequest(const WebViewRouterRequest &) = delete;
  WebViewRouterRequest &operator=(const WebViewRouterRequest &) = delete;

  WebView *webview() const noexcept { return _wvRequest->webview(); }
  const Uri &uri() const noexcept { return _wvRequest->uri(); }

  operator const WebViewRequest &() const {
    return *_wvRequest;
  }
};

} // namespace a8::wv

#endif /* A8_WEBVIEW_REQUEST_H_ */
