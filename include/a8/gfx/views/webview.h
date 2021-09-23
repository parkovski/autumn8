#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "view.h"
#include <string_view>
#include <string>

namespace a8::gfx {

class WebView : public View {
  void *_a = nullptr;
  void *_b = nullptr;

#ifdef _WIN32
  std::wstring
#else
  std::string
#endif
    _initialUrl;

  void initFinished();

public:
  WebView(std::string_view url = {});
  ~WebView();

  void navigate(std::string_view url);

  void init(Window *window) override;
  void resize(RawPoint pos, RawSize size) override;
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_WEBVIEW_H_
