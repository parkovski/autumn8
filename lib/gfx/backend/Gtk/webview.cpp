#include "a8/util/inline.h"
#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"
#include "a8/util/uri.h"
#include "a8/util/mime.types.h"

#define WKWV(o) WEBKIT_WEB_VIEW(*o->nativeHandle())

using namespace a8::gfx;

namespace {
  a8::cstring_view
  getMimeTypeForPath(a8::cstring_view path,
                     a8::cstring_view unknown = "application/octet-stream") {
    static auto const dictionary = a8::getMimeTypeMap(false, true);

    std::string_view pathsv{path};
    auto dot = pathsv.find_last_of('.');
    auto slash = pathsv.find_last_of('/');
#if _WIN32
    auto bkslash = pathsv.find_last_of('\\');
    if (slash == std::string_view::npos || bkslash > slash) {
      slash = bkslash;
    }
#endif

    if ((slash == std::string_view::npos || dot > slash) &&
        dot != std::string_view::npos) {
      auto ext = path.substr(dot + 1);
      auto it = dictionary.find(ext);
      if (it != dictionary.end()) {
        A8_TRACE("Mime type for .{} = {}", ext, it->second);
        return it->second;
      }
      A8_TRACE("Mime type for .{} = ???", ext);
    }

    A8_TRACE("Mime type for path {} defaulting to {}", pathsv, unknown);
    return unknown;
  }

  void webViewSendFile(WebKitURISchemeRequest *req, a8::cstring_view path,
                       a8::cstring_view mimeType, a8::cstring_view origin) {
    auto file = g_file_new_for_path(path.c_str());
    if (!file) {
      auto error = g_error_new(
        webkit_network_error_quark(), WEBKIT_NETWORK_ERROR_FILE_DOES_NOT_EXIST,
        "File '%s' not found", path.c_str());
      webkit_uri_scheme_request_finish_error(req, error);
      return;
    }

    GError *error = nullptr;
    auto stream = G_INPUT_STREAM(g_file_read(file, nullptr, &error));
    if (error) {
      g_error_free(error);
      error = g_error_new(
        webkit_network_error_quark(), WEBKIT_NETWORK_ERROR_FAILED,
        "File '%s' read failed", path.c_str());
      webkit_uri_scheme_request_finish_error(req, error);
      return;
    }

    if (!mimeType) {
      mimeType = getMimeTypeForPath(path.c_str());
    }
    A8_TRACE("Sending {} {}", mimeType, path);
    auto *res = webkit_uri_scheme_response_new(stream, -1);
    auto *headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
    // if (origin.starts_with("a8://app")) {
      soup_message_headers_append(headers, "Access-Control-Allow-Origin",
                                  "a8://app");
    // }
    webkit_uri_scheme_response_set_content_type(res, mimeType);
    webkit_uri_scheme_response_set_http_headers(res, headers);
    webkit_uri_scheme_request_finish_with_response(req, res);
  }

  static void webViewSendResFile(WebKitURISchemeRequest *req,
                                 a8::cstring_view path,
                                 a8::cstring_view mimeType,
                                 a8::cstring_view origin) {
    // TODO Web asset path is app specific.
    auto exeName = a8::platform::getExeName();
    auto fullPath = g_build_filename(A8_WEB_ASSET_PATH, exeName.c_str(),
                                     path.c_str(), nullptr);
    webViewSendFile(req, fullPath, mimeType, origin);
    g_free(fullPath);
  }
} // anonymous namespace

void WebViewResponse::sendFile(cstring_view path) {
  auto req = static_cast<WebKitURISchemeRequest *>(_platformData);
  webViewSendFile(req, path, _contentType, "");
  _state = WroteAllClosed;
}

void WebViewResponse::sendResFile(cstring_view path) {
  auto req = static_cast<WebKitURISchemeRequest *>(_platformData);
  webViewSendResFile(req, path, _contentType, "");
  _state = WroteAllClosed;
}

void WebViewResponse::platformFinish() noexcept {
  if (_state >= WroteAllClosed) {
    return;
  }
  _state = WroteAllClosed;

  auto req = static_cast<WebKitURISchemeRequest *>(_platformData);
  auto content =
#if __cplusplus >= 202002L
    this->_buffer.view();
#else
    this->_buffer.str();
#endif
  auto stream = g_memory_input_stream_new_from_data(
    content.data(), static_cast<gssize>(content.size()), nullptr);
  A8_TRACE("webkit_uri_scheme_request_finish: size = {}, contentType = {}",
           content.size(), _contentType);
  webkit_uri_scheme_request_finish(
    req, stream, static_cast<gint64>(content.size()), _contentType.c_str());
}

WebView::WebView(View *parent)
  : View(parent)
{
  // webkit_website_data_manager_new(
  //   "base-data-directory", "/home/USER/.local/share/webkitgtk",
  //   "base-cache-directory", "/home/USER/.cache/APPNAME");
  // webkit_web_context_new_with_website_data_manager(...);
  auto ctx = webkit_web_context_new();
  webkit_web_context_register_uri_scheme(ctx, CustomScheme.c_str(),
                                         &WebView::onUriSchemeRequest, this,
                                         nullptr);
  auto wkwv = webkit_web_view_new_with_context(ctx);

  initializationFinished(wkwv);
}

WebView::~WebView() {
}

void WebView::navigate(cstring_view url) {
  webkit_web_view_load_uri(WKWV(this), url.c_str());
}

bool WebView::devToolsEnabled() const {
  auto settings = webkit_web_view_get_settings(WKWV(this));
  int enabled;
  g_object_get(G_OBJECT(settings), "enable-developer-extras", &enabled,
               nullptr);
  return (bool)enabled;
}

void WebView::devToolsSetEnabled(bool enable) {
  auto settings = webkit_web_view_get_settings(WKWV(this));
  g_object_set(G_OBJECT(settings), "enable-developer-extras", (int)enable,
               nullptr);
}

bool WebView::devToolsVisible() const {
  auto inspector = webkit_web_view_get_inspector(WKWV(this));
  return inspector && webkit_web_inspector_get_web_view(inspector) != nullptr;
}

bool WebView::devToolsSetVisible(bool show) {
  auto inspector = webkit_web_view_get_inspector(WKWV(this));
  if (!inspector) {
    return false;
  }
  if (show) {
    webkit_web_inspector_show(inspector);
  } else {
    webkit_web_inspector_close(inspector);
  }
  return true;
}

bool WebView::devToolsAttached() const {
  auto inspector = webkit_web_view_get_inspector(WKWV(this));
  if (!inspector) {
    return false;
  }
  return webkit_web_inspector_is_attached(inspector);
}

void WebView::devToolsSetAttached(bool attached) {
  auto inspector = webkit_web_view_get_inspector(WKWV(this));
  if (!inspector) {
    return;
  }
  if (attached) {
    webkit_web_inspector_attach(inspector);
  } else {
    webkit_web_inspector_detach(inspector);
  }
}

void WebView::onUriSchemeRequest(WebKitURISchemeRequest *req, gpointer data)
                                 noexcept {
  auto webview = static_cast<WebView *>(data);

  auto uristr = webkit_uri_scheme_request_get_uri(req);
  A8_DEBUG("WebKitURISchemeRequest {}", uristr);
  Uri uri{uristr};
  if (!uri) {
    auto err = g_error_new(webkit_network_error_quark(),
                           WEBKIT_NETWORK_ERROR_FAILED,
                           "Bad URI '%s'", uristr);
    webkit_uri_scheme_request_finish_error(req, err);
    return;
  }

  if (uri.scheme() != CustomScheme) {
    auto err = g_error_new(webkit_network_error_quark(),
                           WEBKIT_NETWORK_ERROR_UNKNOWN_PROTOCOL,
                           "Unknown protocol '%.*s'",
                           static_cast<int>(uri.scheme().size()),
                           uri.scheme().data());
    webkit_uri_scheme_request_finish_error(req, err);
    return;
  }

  auto mtd = webkit_uri_scheme_request_get_http_method(req);
  auto hdr = webkit_uri_scheme_request_get_http_headers(req);
  auto origin = soup_message_headers_get_one(hdr, "Origin");

  A8_DEBUG("{} from {} -> {}", mtd, origin ? origin : "??", uri.host());
  /*auto res = webkit_uri_scheme_response_new(nullptr, 0);
  auto reshdr = soup_message_headers_new(SOUP_MESSAGE_HEADERS_RESPONSE);
  webkit_uri_scheme_response_set_content_type(res, "text/plain");
  webkit_uri_scheme_response_set_status(res, 200, "OK");
  webkit_uri_scheme_response_set_http_headers(res, reshdr);
  webkit_uri_scheme_request_finish_with_response(req, res);*/

  if (uri.host() == "res") {
    std::string path{uri.path()};
    webViewSendResFile(req, path, {}, origin);
  } else if (uri.host() == "app") {
    WebViewRequest wvreq{webview, std::move(uri)};
    WebViewResponse wvres{req};
    // if (a8::starts_with(origin, "a8://res")) {
      wvres.header("Access-Control-Allow-Origin", "a8://res");
    // }
    if (!webview->callUserCallback(wvreq, wvres)) {
      auto err = g_error_new(webkit_network_error_quark(),
                             WEBKIT_NETWORK_ERROR_FAILED,
                             "User callback failed");
      webkit_uri_scheme_request_finish_error(req, err);
      return;
    }
  }
}