#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "a8/a8.h"
#include "a8/util/cstring_view.h"
#include "a8/util/http.h"

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
private:
  friend class WebView;
  // This is a header, but it's the only required header so it's listed
  // separately.
  std::string _contentType;
  // All the response headers except Content-Type as a CRLF separated string.
  std::string _headers;
  // HTTP status code, e.g. 200.
  int _status;
  // HTTP reason description, e.g. "OK".
  std::string _reason;
  // Response content. Should be a stream.
  std::string _content;

  void finish() {}

public:
  WebViewResponse &contentType(std::string_view type) {
    _contentType = type;
    return *this;
  }

  WebViewResponse &header(std::string_view key, std::string_view value);

  WebViewResponse &status(::a8::http::http_status_t status) {
    _status = status.first;
    _reason = status.second;
    return *this;
  }

  WebViewResponse &write(std::string_view data) {
    _content.append(data);
    return *this;
  }

  void end(std::string_view data = {}) {
    if (data.length()) {
      write(data);
    }
    finish();
  }
};

class WebView : public View {
public:
  constexpr static cstring_view CustomScheme = "a8";
  constexpr static cstring_view ResourceRoot = "a8://res";
  constexpr static cstring_view AppUserRoot = "a8://app";

  using UserCallback = bool(WebView *, const WebViewRequest &,
                            WebViewResponse &);

public:
  explicit WebView(View *parent);
  ~WebView();

  void navigate(cstring_view url);

  void setUserCallback(std::function<UserCallback> callback) {
    _userCallback = callback;
  }

private:
  bool callUserCallback(const WebViewRequest &req, WebViewResponse &res) {
    if (!_userCallback) {
      return false;
    }
    return _userCallback(this, req, res);
  }

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

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_WEBVIEW_H_
