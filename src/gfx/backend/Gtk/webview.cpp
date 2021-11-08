#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

WebView *WebView::create(Window *parent) {
  GtkWidget *wkwv = webkit_web_view_new();
  gtk_container_add(GTK_CONTAINER(parent->nativeHandle()), wkwv);
  return new WebView(wkwv);
}

WebView::WebView(void *webView)
  : _webView{(GtkWidget *)webView}
{
}

WebView::~WebView() {
}

void WebView::navigate(std::string_view url) {
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