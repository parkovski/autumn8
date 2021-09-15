#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "view.h"
#include <string_view>

namespace a8::gfx {

class WebView : public View {
  void *_a = nullptr;
  void *_b = nullptr;

  void initFinished();

public:
  WebView();
  ~WebView();

  void init(Window *window) override;
  void resize(RawPoint pos, RawSize size) override;
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_WEBVIEW_H_
