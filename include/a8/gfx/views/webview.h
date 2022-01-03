#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "view.h"

#include "a8/util/cstring_view.h"
#include "a8/util/http.h"
#include "a8/util/uri.h"
#include "a8/util/router.h"

#include <string_view>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

#if A8_IS_GFX_IMPL(Win32)
# include "WebView2.h"
#elif A8_IS_GFX_IMPL(Gtk)
# include <webkit2/webkit2.h>
#endif

namespace a8::gfx {

class WebView;

struct WebViewRequest {
private:
  WebView *_webview;
  Uri _uri;

public:
  WebViewRequest(WebView *webview, Uri &&uri)
    : _webview{webview}, _uri{std::move(uri)}
  {}

  const WebView *webview() const noexcept { return _webview; }
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

  const WebView *webview() const noexcept { return _wvRequest->webview(); }
  const Uri &uri() const noexcept { return _wvRequest->uri(); }

  operator const WebViewRequest &() const {
    return *_wvRequest;
  }
};

class ResponseStateException : public std::logic_error {
public:
  enum Kind {
    // Tried to write another status after one was already written.
    DuplicateStatus,
    // Called contentType twice.
    DuplicateContentType,
    // Called contentLength twice.
    DuplicateContentLength,
    // Tried to write a header after the content was started.
    HeaderAfterContent,
    // Called end before writing content-length bytes.
    WroteNotEnough,
    // Tried to write more than content-length bytes.
    WroteTooMuch,
    // Called write after the response was closed.
    WriteAfterClose,
    // Called end more than once.
    DuplicateEnd,
  } _kind;

private:
  static const char *kindToString(Kind kind) noexcept;

public:
  explicit ResponseStateException(Kind kind)
    : logic_error{kindToString(kind)},
      _kind{kind}
  {}

  Kind kind() const noexcept {
    return _kind;
  }
};

struct WebViewResponseStreamBase {
  std::stringbuf _buffer;
};

struct WebViewResponse : virtual WebViewResponseStreamBase,
                         public std::ostream {
private:
  friend class WebView;

  void *_platformData;

  enum State {
    WroteNone,           // Can call any methods here.
    WroteStatus,         // Not allowed to change status past this point.
    WroteContentPartial, // Can only write more content past this point.
    WroteContentFull,    // Nothing more can be written.
    WroteAllClosed,      // The response was closed.
  } _state = WroteNone;

  // HTTP status code, e.g. 200.
  int _status = 0;
  // HTTP reason description, e.g. "OK".
  std::string _reason;

  static constexpr size_t ContentLengthUnknown = size_t(-1);
  // Declared content length; optional.
  size_t _contentLength = ContentLengthUnknown;
  // This is a header, but it's the only required header so it's listed
  // separately.
  std::string _contentType;
  // All the response headers except Content-Type as a CRLF separated string.
  std::string _headers;

  // Response content.
  std::stringbuf _buffer;

public:
  WebViewResponse(void *platformData);
  ~WebViewResponse();

  explicit operator bool() const noexcept {
    return _state < WroteAllClosed;
  }

  /// \throws ResponseStateException
  WebViewResponse &status(::a8::http::http_status_t status);

  /// \throws ResponseStateException
  WebViewResponse &contentType(std::string_view type);

  /// \throws ResponseStateException
  WebViewResponse &contentLength(size_t length);

  /// \param key HTTP header name, except `Content-Type` or `Content-Length`.
  /// \param value HTTP header value.
  /// \throws ResponseStateException
  WebViewResponse &header(std::string_view key, std::string_view value);

  /// \throws ResponseStateException
  WebViewResponse &write(std::string_view data);

  /// \throws ResponseStateException
  void end(std::string_view data = {});

  void sendFile(cstring_view path, cstring_view mimeType = {});
  void sendResFile(cstring_view path, cstring_view mimeType = {});

private:
  // Set any missing state to begin writing content.
  void beginWrite() noexcept;
  // Check state consistency after content written.
  void endWrite() noexcept;

  // Platform specific submit request. Should _only_ be called by `endWrite`.
  void finish() noexcept;
};

class WebView : public View {
public:
  constexpr static cstring_view CustomScheme = "a8";
  constexpr static cstring_view ResourceRoot = "a8://res";
  constexpr static cstring_view AppUserRoot = "a8://app";

  using UserCallback = void(const WebViewRequest &, WebViewResponse &);

public:
  explicit WebView(View *parent);
  ~WebView();

  void navigate(cstring_view url);

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

} // namespace a8::gfx

namespace a8 {

template<>
struct RouterTraits<gfx::WebViewRouterRequest> {
  using callback_type = std::function<void(const gfx::WebViewRouterRequest &,
                                           gfx::WebViewResponse &)>;

  static void call(const callback_type &callback,
                   const gfx::WebViewRouterRequest &req,
                   const gfx::WebViewRequest &,
                   gfx::WebViewResponse &res) {
    callback(req, res);
  }

  static gfx::WebViewRouterRequest get_request(const gfx::WebViewRequest &req,
                                               gfx::WebViewResponse &) {
    return gfx::WebViewRouterRequest{&req};
  }

  static std::string_view
  get_route_path(const gfx::WebViewRouterRequest &req) {
    return req.uri().path();
  }

  static RouterBase::RouteParams
  &get_route_params(gfx::WebViewRouterRequest &req) {
    return req;
  }
};

}

#endif // A8_GFX_VIEWS_WEBVIEW_H_
