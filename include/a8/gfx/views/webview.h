#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "view.h"
#include <string_view>

namespace a8::gfx {

class WebView : public View {
  void *_a;
  void *_b;

public:
  WebView();
  WebView(std::string_view url);

  ~WebView();
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_WEBVIEW_H_
