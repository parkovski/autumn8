#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

WebView *WebView::create(Window *parent) {
  GtkWidget *wkwv = webkit_web_view_new();
#if A8_GTK_VERSION == 3
  gtk_container_add(GTK_CONTAINER(parent->nativeHandle()), wkwv);
#elif A8_GTK_VERSION == 4
  gtk_window_set_child(GTK_WINDOW(parent->nativeHandle()), wkwv);
#endif
  return new WebView(wkwv);
}

WebView::WebView(void *webView)
  : _webView{(GtkWidget *)webView}
{
}

WebView::~WebView() {
}

void WebView::navigate(cstring_view url) {
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(_webView), url.c_str());
}

std::string_view WebView::resourceUri() const noexcept {
  return "";
}

std::string_view WebView::userUri() const noexcept {
  return "";
}

void WebView::init(Window *window) {
}

void WebView::resize(RawPoint, RawSize) {
}