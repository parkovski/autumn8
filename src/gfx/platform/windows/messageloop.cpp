#define A8_PLATFORM_WINDOWS_IMPL
#include "a8/a8.h"
#include "a8/platform/windows.h"
#include "a8/gfx/window.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include "fmt/ostream.h"

#include <Windows.h>
#include <string>
#include <codecvt>
#include <future>
#include <atomic>

#define MY_HWND static_cast<HWND>(this->_nativeHandle)
#define HWND_OF(w) static_cast<HWND>((w).nativeHandle())

using namespace a8::platform::windows;
using namespace a8::gfx;

namespace a8::platform::windows {
  const wchar_t *A8WindowClass = L"Autumn8WndClass";
  GlobalPlatformInit g_platformInit;

  std::string to_utf8(std::wstring_view utf16str) {
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
              .to_bytes(&utf16str.front(), &utf16str.back() + 1);
  }

  std::wstring to_utf16(std::string_view utf8str) {
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
              .from_bytes(&utf8str.front(), &utf8str.back() + 1);
  }

  bool setThreadName(const wchar_t *name) {
    static CallSite<decltype(&SetThreadDescription)> fn(
      L"kernel32.dll", "SetThreadDescription"
    );
    return fn && fn(GetCurrentThread(), name) == S_OK;
  }

  DpiStatus enableThreadDpiAwareness() {
    static CallSite<decltype(&SetThreadDpiAwarenessContext)> fn(
      L"user32.dll", "SetThreadDpiAwarenessContext"
    );
    if (fn && fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
      return DpiStatus::Ok;
    }

    static CallSite<HRESULT (WINAPI *)(int)> fnFallback(
      L"shcore.dll", "SetProcessDpiAwareness"
    );
    if (fnFallback && fnFallback(2 /* PROCESS_PER_MONITOR_DPI_AWARE */)) {
      return DpiStatus::Fallback;
    };
    return DpiStatus::Error;
  }
} // namespace a8::platform::windows

Window::Window(std::string_view title)
  : Window()
{
  _nativeHandle = CreateWindowExW(
    WS_EX_ACCEPTFILES,
    A8WindowClass,
    to_utf16(title).c_str(),
    WS_OVERLAPPEDWINDOW,
    0, 0, 500, 500,
    nullptr,
    nullptr,
    g_platformInit.hInstance,
    this
  );

  _size = {500, 500};
  // TODO: Async creation when not in main thread.
}

Window::~Window() {
  if (auto hWnd = MY_HWND; hWnd) {
    DestroyWindow(hWnd);
  }
}

namespace {
  struct MainThreadState {
    Window *window;
    bool running;

    MainThreadState(Window *window) noexcept
      : window(window), running(true)
    {}
  };

  void mainThread(MainThreadState &state) {
    setThreadName(L"Event thread");
    logtrace("Event thread: {}", enableThreadDpiAwareness());

    RECT rc;
    GetClientRect(HWND_OF(*state.window), &rc);
    auto width = rc.right - rc.left;
    auto height = rc.bottom - rc.top;

    bgfx::Init init;
    init.deviceId = 0;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_HIDPI;
    init.type = bgfx::RendererType::Count;
    init.vendorId = BGFX_PCI_ID_NONE;
    if (bgfx::init(init)) {
      if (!state.window->nvgInit()) {
        bgfx::shutdown();
        return;
      }
    } else {
      return;
    }

    bgfx::setDebug(BGFX_DEBUG_TEXT);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, (uint16_t)width, (uint16_t)height);
    bgfx::touch(0);
    bgfx::frame();

    while (state.running) {
      //WaitForSingleObject(state.eventsReady);
      bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
      state.window->render();

      bgfx::frame();
    }

    state.window->nvgDestroy();
    bgfx::shutdown();
  }
}

static RawSize toSize(LPRECT rect) {
  return {(unsigned)(rect->right - rect->left), (unsigned)(rect->bottom - rect->top)};
}

static RawSize getNonClientBorders(HWND hWnd) {
  WINDOWINFO wi;
  wi.cbSize = sizeof(WINDOWINFO);
  GetWindowInfo(hWnd, &wi);
  return toSize(&wi.rcWindow) - toSize(&wi.rcClient);
}

LRESULT CALLBACK
a8::platform::windows::mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  auto *window = reinterpret_cast<Window *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

  switch (msg) {
  case WM_CREATE:
    {
      auto *createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
      window = reinterpret_cast<Window *>(createStruct->lpCreateParams);
      SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
      window->_scale =
        GetDpiForWindow(hWnd) / (float)USER_DEFAULT_SCREEN_DPI;
      window->postEvent(SystemEvent::Initialize);
    }
    break;
  case WM_CLOSE:
    window->postEvent(SystemEvent::Close);
    return 0;
  case WM_DESTROY:
    window->postEvent(SystemEvent::Destroy);
    return 0;
  case WM_NCDESTROY:
    return 0;
  case WM_MOUSEMOVE:
    {
      int x = LOWORD(lParam);
      int y = HIWORD(lParam);
      // TODO: Are any buttons down?
      window->postEvent(
        RawPointerEvent(
          {x, y}, 1, PointerKind::Mouse, PointerAction::Moved, false, 0, 0
        )
      );
    }
    break;
  case WM_ERASEBKGND:
    break;
  case WM_PAINT:
    break;
  case WM_SIZING:
    {
      Anchor side = Anchor::None;
      switch (wParam) {
      case WMSZ_LEFT:
        side = Anchor::Left;
        break;
      case WMSZ_TOPLEFT:
        side = Anchor::TopLeft;
        break;
      case WMSZ_TOP:
        side = Anchor::Top;
        break;
      case WMSZ_TOPRIGHT:
        side = Anchor::TopRight;
        break;
      case WMSZ_RIGHT:
        side = Anchor::Right;
        break;
      case WMSZ_BOTTOMRIGHT:
        side = Anchor::BottomRight;
        break;
      case WMSZ_BOTTOM:
        side = Anchor::Bottom;
        break;
      case WMSZ_BOTTOMLEFT:
        side = Anchor::BottomLeft;
        break;
      default:
        break;
      }
      RECT *rc = reinterpret_cast<RECT *>(lParam);
      window->postEvent(RawSizingEvent(toSize(rc) - getNonClientBorders(hWnd), side));
    }
    return TRUE;
  case WM_MOVING:
    {
      auto rc = reinterpret_cast<LPRECT>(lParam);
      window->postEvent(RawMovingEvent{{rc->left, rc->top}});
    }
    break;
  case WM_ENTERSIZEMOVE:
  case WM_EXITSIZEMOVE:
    break;
  //case WM_WINDOWPOSCHANGING:
  case WM_WINDOWPOSCHANGED:
    {
      auto pos = reinterpret_cast<LPWINDOWPOS>(lParam);
      const RawSize size{
        RawSize{(unsigned)pos->cx, (unsigned)pos->cy} - getNonClientBorders(hWnd)
      };
      const RawPoint point{pos->x, pos->y};
      switch (pos->flags & (SWP_NOMOVE | SWP_NOSIZE)) {
      case 0:
        //logtrace("{}: {}, {} : {}x{}", name, pos->x, pos->y, pos->cx, pos->cy);
        window->postEvents(RawSizingEvent{size, Anchor::None}, RawMovingEvent{point});
        break;
      case SWP_NOMOVE:
        //logtrace("{}: {}, {}", name, pos->cx, pos->cy);
        window->postEvents(RawSizingEvent{size, Anchor::None});
        break;
      case SWP_NOSIZE:
        //logtrace("{}: {}x{}", name, pos->x, pos->y);
        window->postEvent(RawMovingEvent{point});
        break;
      default:
        //logtrace("{}", name);
        break;
      }
    }
    return 0;
  case WM_SETFOCUS:
    window->postEvent(SystemEvent::Focus);
    break;
  case WM_KILLFOCUS:
    window->postEvent(SystemEvent::Unfocus);
    break;
  case WM_DPICHANGED:
    {
      auto scale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
      auto rect = reinterpret_cast<LPRECT>(lParam);
      auto x = rect->left;
      auto y = rect->top;
      auto w = rect->right - x;
      auto h = rect->bottom - y;
      SetWindowPos(hWnd, nullptr, x, y, w, h, SWP_NOZORDER);
      auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
      MONITORINFOEXW mi{};
      mi.cbSize = sizeof(MONITORINFOEXW);
      GetMonitorInfoW(monitor, &mi);
      auto xres = mi.rcMonitor.right - mi.rcMonitor.left;
      auto yres = mi.rcMonitor.bottom - mi.rcMonitor.top;
      window->postEvent(DisplayScaleEvent(scale, {(uint32_t)xres, (uint32_t)yres}));
    }
    break;
  }

  return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int a8::gfx::runMessageLoop(Window &window) {
  g_platformInit.dwMsgThreadId = GetCurrentThreadId();

  bgfx::PlatformData platformData{};
  platformData.nwh = window.nativeHandle();
  bgfx::setPlatformData(platformData);

  ShowWindow(HWND_OF(window), g_platformInit.nCmdShow);
  UpdateWindow(HWND_OF(window));

  // Tell it we're going to use our own render thread.
  bgfx::renderFrame();

  MainThreadState state{&window};
  std::thread thread(mainThread, std::ref(state));

  constexpr ULONGLONG interval = 160'000;
  ULONGLONG currentTime;
  QueryInterruptTime(&currentTime);
  ULONGLONG nextTime = currentTime + interval;

  bgfx::renderFrame();

  MSG msg;
  while (true) {
    QueryInterruptTime(&currentTime);
    if (currentTime < nextTime) {
      auto ms = (DWORD)((nextTime - currentTime) / 10'000ULL);
      if (MsgWaitForMultipleObjects(0, nullptr, false, ms, QS_ALLINPUT) != WAIT_TIMEOUT) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
          TranslateMessage(&msg);
          DispatchMessageW(&msg);
          if (msg.message == WM_QUIT) {
            goto break_message_loop;
          }
        }
      }
      QueryInterruptTime(&currentTime);
    }
    if (currentTime > nextTime) {
      nextTime = currentTime + interval;
      bgfx::renderFrame();
    }
  }
break_message_loop:

  state.running = false;

  while (bgfx::renderFrame() != bgfx::RenderFrame::NoContext) {}

  thread.join();

  return (int)msg.wParam;
}

void a8::gfx::postQuitEvent(int exitCode) {
  PostThreadMessageW(g_platformInit.dwMsgThreadId, WM_QUIT, exitCode, 0);
}
