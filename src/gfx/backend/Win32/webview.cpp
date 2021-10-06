#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/views/webview.h"
#include "a8/gfx/window.h"
#include "a8/util/utfconv.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <wrl.h>
#include <wil/com.h>

using namespace a8::gfx;
using namespace Microsoft::WRL;

WebView::WebView(std::string_view url)
  : _controller{nullptr},
    _webView{nullptr},
    _initialUrl{url}
{}

WebView::~WebView() {
  if (_controller) {
    _controller->Release();
  }
  if (_env) {
    _env->Release();
  }
}

static constexpr std::wstring_view AppDomain = L"apprc.parkovski.com";

#define LOGHR(Msg, X) if (auto _hr_ = (X); FAILED(_hr_)) a8::Log::error(Msg ": 0x{:X}", (unsigned)_hr_)

void WebView::initFinished() {
  using namespace std::string_view_literals;
  EventRegistrationToken resourceRequestedToken;
  _webView->add_WebResourceRequested(
    Callback<ICoreWebView2WebResourceRequestedEventHandler>(
      [this](ICoreWebView2 *sender,
             ICoreWebView2WebResourceRequestedEventArgs *args) -> HRESULT {
        ICoreWebView2WebResourceRequest *request;
        ICoreWebView2WebResourceResponse *response;
        args->get_Request(&request);
        wchar_t *uri;
        request->get_Uri(&uri);
        a8::Log::debug(L"Requested URI {}", uri);
        wchar_t *uriDomain = nullptr;
        if (a8::starts_with(uri, L"http"sv)) {
          if (a8::starts_with(uri + 4, L"://"sv)) {
            // http
            uriDomain = uri + 7;
          } else if (a8::starts_with(uri + 4, L"s://"sv)) {
            // https
            uriDomain = uri + 8;
          }
        }
        if (uriDomain && a8::starts_with(uriDomain, AppDomain)) {
          // it's a file url
          std::wstring filepath{L"C:\\dev\\autumn8\\src\\tools"};
          filepath.append(uriDomain + AppDomain.length());
          a8::Log::debug(L"File: {}", filepath);
          IStream *stream;
          RETURN_IF_FAILED(SHCreateStreamOnFileEx(filepath.c_str(),
                                                  STGM_READ | STGM_SHARE_DENY_WRITE,
                                                  FILE_FLAG_SEQUENTIAL_SCAN,
                                                  false, nullptr, &stream));
          const wchar_t *headers =
            L"Content-Type: text/html\r\n"
            L"Cache-Control: no-store, max-age=0\r\n";
          LOGHR("CreateWebResourceResponse",
                _env->CreateWebResourceResponse(stream, 200, L"OK", headers, &response));
          LOGHR("put_Response", args->put_Response(response));
        }
        return S_OK;
      }).Get(),
    &resourceRequestedToken
  );
  _webView->AddWebResourceRequestedFilter(
    L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

  if (_initialUrl.size()) {
    navigate(_initialUrl);
  }
}

void WebView::navigate(std::string_view url) {
  if (!_webView) {
    return;
  }
  std::wstring urlw;
  to_utf16(urlw, url);
  auto result = _webView->Navigate(urlw.c_str());
  if (result != S_OK) {
    a8::Log::error("WebView Navigate: 0x{:X}", (unsigned)result);
  }
}

void WebView::init(Window *window) {
  Log::debug("WebView init");
  auto hwnd = static_cast<HWND>(window->nativeHandle());

  /*std::wstring*/ const wchar_t *userDataFolder = nullptr;
  CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr,
    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [this, hwnd](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
        if (FAILED(result)) {
          Log::error("CreateCoreWebView2EnvironmentWithOptions: 0x{:X}.",
                     result);
          return result;
        }

        env->AddRef();
        _env = env;

        env->CreateCoreWebView2Controller(hwnd,
          Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [this, hwnd](HRESULT result, ICoreWebView2Controller *controller)
                        -> HRESULT {
              if (FAILED(result)) {
                Log::error("CreateCoreWebView2Controller: 0x{:X}.", result);
                return result;
              }

              controller->AddRef();
              this->_controller = controller;

              result = controller->get_CoreWebView2(&this->_webView);
              if (FAILED(result)) {
                Log::error("get_CoreWebView2: 0x{:X}", result);
                return result;
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
  if (!_webView) { return; }
  RECT rc;
  rc.left = pos.x;
  rc.top = pos.y;
  rc.right = size.width - pos.x;
  rc.bottom = size.height - pos.y;
  _controller->put_Bounds(rc);
}
