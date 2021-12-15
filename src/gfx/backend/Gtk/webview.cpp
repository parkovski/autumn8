#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"
#include "uriparser/Uri.h"

using namespace a8::gfx;

namespace {
  std::string_view sv(UriTextRangeA range) noexcept {
    return {range.first, static_cast<size_t>(range.afterLast - range.first)};
  }
  std::string fullpath(UriPathSegmentA *head) {
    std::string path;
    while (head) {
      path.append(1, '/').append(sv(head->text));
      head = head->next;
    }
    return path;
  }
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
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(*nativeHandle()), url.c_str());
}

void WebView::onUriSchemeRequest(WebKitURISchemeRequest *req, gpointer data)
                                 noexcept {
  auto webview = static_cast<WebView *>(data);

  auto uristr = webkit_uri_scheme_request_get_uri(req);
  a8::Log::debug("WebKitURISchemeRequest {}", uristr);
  const char *errorPos;
  UriUriA uri;
  if (uriParseSingleUriA(&uri, uristr, &errorPos) != URI_SUCCESS) {
    auto err = g_error_new(webkit_network_error_quark(),
                           WEBKIT_NETWORK_ERROR_FAILED,
                           "Bad URI '%s'", uristr);
    webkit_uri_scheme_request_finish_error(req, err);
    return;
  }
  Log::debug("URI {{ scheme: {}, host: {}, path: {} }}", sv(uri.scheme),
             sv(uri.hostText), fullpath(uri.pathHead));
  if (sv(uri.scheme) != CustomScheme) {
    auto err = g_error_new(webkit_network_error_quark(),
                           WEBKIT_NETWORK_ERROR_UNKNOWN_PROTOCOL,
                           "Unknown protocol '%.*s'",
                           static_cast<int>(uri.scheme.afterLast -
                                            uri.scheme.first),
                           uri.scheme.afterLast);
  }

  auto host = sv(uri.hostText);
  auto path = fullpath(uri.pathHead);
  if (starts_with(host, "res")) {
    auto file = g_file_new_build_filename(
      "/home/parker/Documents/dev/autumn8/src/tools", path.c_str(), nullptr);
    auto stream = G_INPUT_STREAM(g_file_read(file, nullptr, nullptr));
    const char *contentType = "text/plain";
    if (a8::ends_with(path, ".html")) {
      contentType = "text/html";
    } else if (a8::ends_with(path, ".js")) {
      contentType = "application/javascript";
    }
    webkit_uri_scheme_request_finish(req, stream, -1, contentType);
  } else if (starts_with(host, "app")) {
    WebViewRequest wvreq;
    wvreq.method = "GET";
    wvreq.uri = path;
    WebViewResponse wvres;
    if (!webview->callUserCallback(wvreq, wvres)) {
      auto err = g_error_new(webkit_network_error_quark(),
                             WEBKIT_NETWORK_ERROR_FAILED, "");
      webkit_uri_scheme_request_finish_error(req, err);
      return;
    }

    // TODO: Async
    auto bytes = g_bytes_new(wvres._content.data(), wvres._content.size());
    auto stream = g_memory_input_stream_new_from_bytes(bytes);
    webkit_uri_scheme_request_finish(
      req, stream, static_cast<gint64>(wvres._content.size()),
      wvres._contentType.c_str());
  }
}