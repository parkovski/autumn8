#ifndef A8_WEBVIEW_WEBVIEW_H_
#define A8_WEBVIEW_WEBVIEW_H_

#include "request.h"
#include "response.h"

#include "a8/window/view.h"

#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>

#if A8_IS_GFX_IMPL(Win32)
# include "WebView2.h"
#elif A8_IS_GFX_IMPL(Gtk)
# include <webkit2/webkit2.h>
#endif

namespace a8::wv {

class WebView;

class WebView : public win::View {
public:
  constexpr static cstring_view CustomScheme = "a8";
  constexpr static cstring_view ResourceRoot = "a8://res";
  constexpr static cstring_view AppUserRoot = "a8://app";

  using UserCallback = void(const WebViewRequest &, WebViewResponse &);

public:
  explicit WebView(View *parent);
  ~WebView();

  void navigate(cstring_view url);

  bool devToolsEnabled() const;
  void devToolsSetEnabled(bool enable = true);

  bool devToolsVisible() const;
  bool devToolsSetVisible(bool show);

  bool devToolsAttached() const;
  void devToolsSetAttached(bool attached);

  template<class Callback>
  void setUserCallback(Callback &&callback) {
    _userCallback = std::forward<Callback>(callback);
  }

private:
  bool callUserCallback(const WebViewRequest &req, WebViewResponse &res)
                        noexcept;

  std::function<UserCallback> _userCallback;

#if A8_IS_GFX(Win32)
# ifndef A8_INCLUDE_GFX_BACKEND
  class ICoreWebView2Environment;
  class ICoreWebView2Controller;
  class ICoreWebView2;
# endif

  ICoreWebView2Environment *_env;
  ICoreWebView2Controller *_controller;
  ICoreWebView2 *_webView;
#elif A8_IS_GFX_IMPL(Gtk)
  static void onUriSchemeRequest(WebKitURISchemeRequest *req, gpointer data)
                                 noexcept;
#endif
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

  static RouterBase::RouteParams
  &get_route_params(wv::WebViewRouterRequest &req) {
    return req;
  }
};

} // namespace a8

#endif // A8_WEBVIEW_WEBVIEW_H_
