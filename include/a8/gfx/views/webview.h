#ifndef A8_GFX_VIEWS_WEBVIEW_H_
#define A8_GFX_VIEWS_WEBVIEW_H_

#include "a8/a8.h"

#include "view.h"
#include <string_view>
#include <string>

#if A8_IS_GFX_IMPL(Win32)
# include "WebView2.h"
#endif

namespace a8::gfx {

class WebView : public View {
#if A8_IS_GFX(Win32)
# ifdef A8_INCLUDE_GFX_BACKEND
  ICoreWebView2Environment *_env;
  ICoreWebView2Controller *_controller;
  ICoreWebView2 *_webView;
# else
  void *_env;
  void *_controller;
  void *_webView;
# endif
#endif

  std::string _initialUrl;

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
