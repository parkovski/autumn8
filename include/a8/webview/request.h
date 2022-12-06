#ifndef A8_WEBVIEW_REQUEST_H_
#define A8_WEBVIEW_REQUEST_H_

#include "a8/util/uri.h"
#include "a8/util/router.h"

namespace a8::wv {

class WebView;
class WebViewResponse;

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

namespace a8 {

template<>
struct RouterTraits<wv::WebViewRouterRequest> {
  using callback_type
    = std::function<void(const wv::WebViewRouterRequest &,
                         wv::WebViewResponse &)>;

  static void call(const callback_type &callback,
                   const wv::WebViewRouterRequest &req,
                   const wv::WebViewRequest &,
                   wv::WebViewResponse &res) {
    callback(req, res);
  }

  static wv::WebViewRouterRequest
  get_request(const wv::WebViewRequest &req,
              wv::WebViewResponse &) {
    return wv::WebViewRouterRequest{&req};
  }

  static std::string_view
  get_route_path(const wv::WebViewRouterRequest &req) {
    return req.uri().path();
  }

  static RouterBase::RouteParams &
  get_route_params(wv::WebViewRouterRequest &req) {
    return req;
  }
};

} // namespace a8

#endif /* A8_WEBVIEW_REQUEST_H_ */
