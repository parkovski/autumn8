#include "a8/platform/Windows/platform.h"
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"

#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"

#define CONTROLLERP(V) static_cast<ICoreWebView2Controller *>((V)._a)
#define WINDOWP(V) static_cast<ICoreWebView2 *>((V)._b)

using namespace a8::gfx;
using namespace Microsoft::WRL;

WebView::WebView() {
  RoInitialize(RO_INIT_SINGLETHREADED);
}

WebView::~WebView() {
  if (_b) {
    WINDOWP(*this)->Release();
  }
  if (_a) {
    CONTROLLERP(*this)->Release();
  }

  RoUninitialize();
}

void WebView::initFinished() {
  if (!_a || !_b) {
    return;
  }

  Log::debug("WebView navigate");
  WINDOWP(*this)->Navigate(L"https://www.bing.com/");
}

void WebView::init(Window *window) {
  Log::debug("WebView init");
  auto hwnd = static_cast<HWND>(window->nativeHandle());
  Log::debug("Handle = 0x{:X}", (size_t)hwnd);

  CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
    [hwnd, this](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
      Log::debug("CreateCoreWebView2EnvironmentWithOptions: 0x{:X}.", result);
      env->CreateCoreWebView2Controller(hwnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
        [this, hwnd](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
          Log::debug("CreateCoreWebView2Controller: 0x{:X}.", result);
          if (controller) {
            controller->AddRef();
            this->_a = controller;
            controller->get_CoreWebView2(reinterpret_cast<ICoreWebView2 **>(&this->_b));
          } else {
            Log::warn("WebView no controller!");
          }
          RECT bounds;
          GetClientRect(hwnd, &bounds);
          controller->put_Bounds(bounds);
          this->initFinished();
          return S_OK;
        }).Get());

      return S_OK;
    }).Get());
}

void WebView::resize(RawPoint pos, RawSize size) {
  Log::debug("WebView resize: {} x {}", pos, size);
  if (!_a || !_b) { return; }
  auto c = CONTROLLERP(*this);
  RECT rc;
  rc.left = pos.x;
  rc.top = pos.y;
  rc.right = size.width - pos.x;
  rc.bottom = size.height - pos.y;
  c->put_Bounds(rc);
}
