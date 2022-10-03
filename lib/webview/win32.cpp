#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/webview/webview.h"
#include "a8/util/utfconv.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <wrl.h>
#include <wil/com.h>

using namespace a8::webview;
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

static constexpr std::string_view RsrcUriBase = "http://appres.a8.parkovski.com";
static constexpr std::wstring_view RsrcDomain = L"appres.a8.parkovski.com";

static constexpr std::string_view UserUriBase = "http://appusr.a8.parkovski.com";
static constexpr std::wstring_view UserDomain = L"appusr.a8.parkovski.com";

#define LOGHR(Msg, X) if (auto _hr_ = (X); FAILED(_hr_)) A8_ERROR(Msg ": 0x{:X}", (unsigned)_hr_)

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
        A8_DEBUG(L"Requested URI {}", uri);
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
        if (uriDomain) {
          if (a8::starts_with(uriDomain, RsrcDomain)) {
            // App resource request.
            // TODO: Look up the app's actual path or allow configuring this.
            std::wstring filepath{L"C:\\dev\\autumn8\\src\\tools"};
            filepath.append(uriDomain + RsrcDomain.length());
            A8_DEBUG(L"File: {}", filepath);
            IStream *stream;
            RETURN_IF_FAILED(
              SHCreateStreamOnFileEx(filepath.c_str(),
                                     STGM_READ | STGM_SHARE_DENY_WRITE,
                                     FILE_FLAG_SEQUENTIAL_SCAN,
                                     false, nullptr, &stream)
            );
            // TODO: Find the actual content type.
            const wchar_t *headers =
              L"Content-Type: text/html\r\n"
              L"Cache-Control: no-store, max-age=0\r\n";
            LOGHR("CreateWebResourceResponse",
                  _env->CreateWebResourceResponse(stream, 200, L"OK", headers, &response));
            LOGHR("put_Response", args->put_Response(response));
          } else if (a8::starts_with(uriDomain, UserDomain)) {
            // User resource request.
            WebViewRequest wvreq;
            WebViewResponse wvres;

            if (_userCallback) {
              wchar_t *method;
              request->get_Method(&method);
              a8::to_utf8(wvreq.method, method);

              // TODO: Use only the path.
              a8::to_utf8(wvreq.uri, uri);

              // TODO: Fill in headers.
              ICoreWebView2HttpRequestHeaders *headers;
              request->get_Headers(&headers);
              ICoreWebView2HttpHeadersCollectionIterator *iterator;
              headers->GetIterator(&iterator);
              wchar_t *headerName, *headerValue;
              while (iterator->GetCurrentHeader(&headerName, &headerValue) == S_OK) {
                wvreq.headers.emplace(a8::to_utf8(headerName), a8::to_utf8(headerValue));
                BOOL hasNext;
                iterator->MoveNext(&hasNext);
                if (!hasNext) {
                  break;
                }
              }

              if (!_userCallback(this, wvreq, wvres)) {
                A8_DEBUG("User response: false; 404 Not Found.");
                wvres.status = 404;
                wvres.reason = "Not Found";
                wvres.content.clear();
                wvres.headers.clear();
                wvres.contentType.clear();
              } else {
                A8_DEBUG("User response: {} {}; {}; {}B.",
                         wvres.status, wvres.reason, wvres.contentType,
                         wvres.content.size());
              }
            } else {
              wvres.status = 501;
              wvres.reason = "Not Implemented";
              A8_DEBUG("User response: null; 501 Not Implemented.");
            }

            std::wstring headers{L"Content-Type: "};
            if (wvres.contentType.empty()) {
              headers.append(L"text/plain\r\n");
            } else {
              a8::to_utf16(headers, wvres.contentType);
              headers.append(L"\r\n");
            }
            a8::to_utf16(headers, wvres.headers);

            std::wstring reason = a8::to_utf16(wvres.reason);

            IStream *stream = SHCreateMemStream(
              reinterpret_cast<const BYTE *>(wvres.content.data()),
              static_cast<UINT>(wvres.content.length()));

            _env->CreateWebResourceResponse(stream, wvres.status,
                                            reason.c_str(), headers.c_str(),
                                            &response);
            args->put_Response(response);
          }
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
    A8_ERROR("WebView Navigate: 0x{:X}", (unsigned)result);
  }
}

std::string_view WebView::resourceUri() const noexcept {
  return RsrcUriBase;
}

std::string_view WebView::userUri() const noexcept {
  return UserUriBase;
}

void WebView::init(Window *window) {
  A8_DEBUG("WebView init");
  auto hwnd = static_cast<HWND>(window->nativeHandle());

  /*std::wstring*/ const wchar_t *userDataFolder = nullptr;
  CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder, nullptr,
    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [this, hwnd](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
        if (FAILED(result)) {
          A8_ERROR("CreateCoreWebView2EnvironmentWithOptions: 0x{:X}.",
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
                A8_ERROR("CreateCoreWebView2Controller: 0x{:X}.", result);
                return result;
              }

              controller->AddRef();
              this->_controller = controller;

              result = controller->get_CoreWebView2(&this->_webView);
              if (FAILED(result)) {
                A8_ERROR("get_CoreWebView2: 0x{:X}", result);
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
