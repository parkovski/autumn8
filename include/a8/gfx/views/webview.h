#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "a8/a8.h"
#include "a8/util/cstring_view.h"

#include "view.h"
#include <string_view>
#include <string>
#include <unordered_map>

#if A8_IS_GFX_IMPL(Win32)
# include "WebView2.h"
#elif A8_IS_GFX_IMPL(Gtk)
# include <webkit2/webkit2.h>
#endif

namespace a8::gfx {

struct WebViewRequest {
  std::unordered_map<std::string, std::string> headers;
  std::string method;
  std::string uri;
  //std::string_view scheme;
  //std::string_view userinfo;
  //std::string_view host;
  //uint16_t port;
  //std::string_view path;
  //std::string_view query;
  //std::string_view fragment;
};

struct WebViewResponse {
  // This is a header, but it's the only required header so it's listed
  // separately.
  std::string contentType;
  // All the response headers except Content-Type as a CRLF separated string.
  std::string headers;
  // HTTP status code, e.g. 200.
  int status;
  // HTTP reason description, e.g. "OK".
  std::string reason;
  // Response content. Should be a stream.
  std::string content;
};

class WebView : public View {
public:
  using UserCallback = bool(WebView *, const WebViewRequest &,
                            WebViewResponse &);

private:
#if A8_IS_GFX(Win32)
# ifdef A8_INCLUDE_GFX_BACKEND
  ICoreWebView2Environment *_env;
  ICoreWebView2Controller *_controller;
  ICoreWebView2 *_webView;
# else
  void *_env;
  void *_controller;
  void *_webView;
# endif
#endif

  std::string _userScheme;
  std::function<UserCallback> _userCallback;

public:
  explicit WebView(View *parent);
  ~WebView();

  //void setUserScheme(cstring_view scheme);

  void navigate(cstring_view url);

  // The URL where app resources can be found on disk.
  std::string_view resourceUri() const noexcept;

  // The URL that triggers the user callback.
  std::string_view userUri() const noexcept;

  void setUserCallback(std::function<UserCallback> callback) {
    _userCallback = callback;
  }

  bool callUserCallback(const WebViewRequest &req, WebViewResponse &res) {
    if (!_userCallback) {
      return false;
    }
    return _userCallback(this, req, res);
  }
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_WEBVIEW_H_
