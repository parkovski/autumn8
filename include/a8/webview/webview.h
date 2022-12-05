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

#endif // A8_WEBVIEW_WEBVIEW_H_
