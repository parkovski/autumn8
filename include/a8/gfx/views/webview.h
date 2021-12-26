#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "view.h"

#include "a8/util/cstring_view.h"
#include "a8/util/http.h"
#include "a8/util/uri.h"

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

struct WebViewRequest {
  std::unordered_map<std::string, std::string> headers;
  std::string method;
  Uri uri;
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

  using UserCallback = void(WebView *, const WebViewRequest &,
                            WebViewResponse &);

public:
  explicit WebView(View *parent);
  ~WebView();

  void navigate(cstring_view url);

  void setUserCallback(std::function<UserCallback> callback) {
    _userCallback = callback;
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

#endif // A8_GFX_VIEWS_WEBVIEW_H_
