#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"

using namespace a8::gfx;

WebView::WebView(View *parent)
  : View(parent)
{
  auto wkwv = webkit_web_view_new();
  initializationFinished(wkwv);
}

WebView::~WebView() {
}

void WebView::navigate(cstring_view url) {
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(*nativeHandle()), url.c_str());
}

std::string_view WebView::resourceUri() const noexcept {
  return "";
}

std::string_view WebView::userUri() const noexcept {
  return "";
}