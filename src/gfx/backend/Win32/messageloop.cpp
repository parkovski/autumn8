#define A8_PLATFORM_WINDOWS_IMPL

#include "a8/a8.h"
#include "a8/platform/Windows/platform.h"
#include "a8/gfx/window.h"
#include "a8/util/utfconv.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include "fmt/ostream.h"

#include <Windows.h>
#include <string>
#include <future>
#include <atomic>
#include <ostream>

#define MY_HWND static_cast<HWND>(this->_nativeHandle)
#define HWND_OF(w) static_cast<HWND>((w).nativeHandle())

using a8::Log;
using namespace a8::platform::windows;
using namespace a8::gfx;
using namespace a8::util::utfconv;

namespace a8::platform::windows {

const wchar_t *A8WindowClass = L"Autumn8WndClass";
GlobalPlatformInit g_platformInit;

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

namespace a8::gfx {
  void default_render(View &, Renderer &);
}
using a8::gfx::default_render;

Window::Window() noexcept
  : _events{QueueSize}, _nvg{}, _rootView{std::make_unique<EmptyView>(this)},
    _scale{1}, _zoom{1}, _pos{}, _size{}
{}

Window::Window(std::string_view title, Point pos, Size size) noexcept
  : Window()
{
  _pos = RawPoint{pos};
  _size = RawSize{size};

  Log::info("Creating window at {}, size {}, scale {}", _pos, _size, _scale);

  _nativeHandle = CreateWindowExW(
    WS_EX_ACCEPTFILES,
    A8WindowClass,
    to_utf16(title).c_str(),
    WS_OVERLAPPEDWINDOW,
    _pos.x, _pos.y, _size.width, _size.height,
    nullptr,
    nullptr,
    g_platformInit.hInstance,
    this
  );
  // TODO: Async creation when not in main thread.
}

Window::Window(const Window::native_handle_t &, void *nativeHandle) noexcept
  : Window()
{
  _nativeHandle = nativeHandle;
  HWND hWnd = static_cast<HWND>(nativeHandle);
  RECT rc;
  if (GetWindowRect(hWnd, &rc)) {
    _pos = {rc.left, rc.top};
    _size = {
      rc.right - rc.left,
      rc.bottom - rc.top
    };
  }
  _scale = GetDpiForWindow(hWnd) / (float)USER_DEFAULT_SCREEN_DPI;
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
    Log::trace("Main thread: {}", enableThreadDpiAwareness());

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
      state.window->render();
      bgfx::frame();
    }

    state.window->nvgDestroy();
    bgfx::shutdown();
  }
}

static RawSize toSize(LPRECT rect) {
  return {(rect->right - rect->left), (rect->bottom - rect->top)};
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

      window->_scale = GetDpiForWindow(hWnd) / (float)USER_DEFAULT_SCREEN_DPI;
      window->_pos = RawPoint{Point{window->_pos} * window->_scale};
      window->_size = RawSize{Size{window->_size} * window->_scale};
      SetWindowPos(hWnd, nullptr, window->_pos.x, window->_pos.y,
                   window->_size.width, window->_size.height,
                   SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
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
    return 0;

  case WM_PAINT:
      /*if (window->_nvg) {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        window->render();
        EndPaint(hWnd, &ps);
        return 0;
      }*/
    break;
#if 0
  case WM_SIZE:
    {
      RawSize size{(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
      Log::debug("WM_SIZE; wParam = 0x{:X}", wParam);
      switch (wParam) {
      case SIZE_MINIMIZED:
        window->postEvent(SystemEvent::Minimize);
        break;
      case SIZE_MAXIMIZED:
        window->postEvents(RawResizeEvent{size}, SystemEvent::Maximize);
        break;
      // case SIZE_RESTORED:
      //   window->postEvents(RawResizeEvent{size}, SystemEvent::RestoreSize);
      //   break;
      default:
        window->postEvent(RawResizeEvent{size});
        break;
      }
    }
    return 0;

  case WM_SIZING:
    Log::trace("WM_SIZING");
    {
      Anchor side;
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
        side = Anchor::None;
        break;
      }

      RECT *rc = reinterpret_cast<RECT *>(lParam);
      auto size = toSize(rc) - getNonClientBorders(hWnd);
      //window->postEvent(RawSizingEvent{size, side});
    }
    return true;

  case WM_MOVE:
    {
      int x = (int)(short)LOWORD(lParam);
      int y = (int)(short)HIWORD(lParam);
      window->postEvent(RawMovingEvent{RawPoint{x, y}});
    }
    return 0;

  case WM_MOVING:
    {
      auto rc = reinterpret_cast<LPRECT>(lParam);
      window->postEvent(RawMovingEvent{{rc->left, rc->top}});
    }
    return true;
#endif
  case WM_ENTERSIZEMOVE:
    Log::trace("WM_ENTERSIZEMOVE");
    break;

  case WM_EXITSIZEMOVE:
    Log::trace("WM_EXITSIZEMOVE");
    break;

  case WM_WINDOWPOSCHANGING:
    {
      auto pos = reinterpret_cast<LPWINDOWPOS>(lParam);
      const RawPoint point{pos->x, pos->y};
      const RawSize borderSize = getNonClientBorders(hWnd);
      RawSize size{pos->cx, pos->cy};
      if (size.width > 0 && size.height > 0) {
        size -= borderSize;
      }
      Log::trace("WM_WINDOWPOSCHANGING: {}, {} : {}x{}, flags=0x{:X}",
                 pos->x, pos->y, pos->cx, pos->cy, pos->flags);
      switch (pos->flags & (SWP_NOMOVE | SWP_NOSIZE)) {
      case 0:
        window->postEvents(RawSizingEvent{size, Anchor::None}, RawMovingEvent{point});
        break;
      case SWP_NOMOVE:
        window->postEvent(RawSizingEvent{size, Anchor::None});
        break;
      case SWP_NOSIZE:
        window->postEvent(RawMovingEvent{point});
        break;
      default:
        break;
      }
    }
    return 0;

  case WM_WINDOWPOSCHANGED:
    {
      auto pos = reinterpret_cast<LPWINDOWPOS>(lParam);
      const RawSize size{
        RawSize{pos->cx, pos->cy} - getNonClientBorders(hWnd)
      };
      const RawPoint point{pos->x, pos->y};
      Log::trace("WM_WINDOWPOSCHANGED: {}, {} : {}x{}; flags=0x{:X}",
                 pos->x, pos->y, pos->cx, pos->cy, pos->flags);
      switch (pos->flags & (SWP_NOMOVE | SWP_NOSIZE)) {
      case 0:
        window->postEvents(RawResizeEvent{size}, RawMovingEvent{point});
        break;
      case SWP_NOMOVE:
        //Log::trace("WindowPos: {}, {}", pos->cx, pos->cy);
        window->postEvents(RawResizeEvent{size});
        break;
      case SWP_NOSIZE:
        //Log::trace("WindowPos: {}x{}", pos->x, pos->y);
        window->postEvent(RawMovingEvent{point});
        break;
      default:
        //Log::trace("WindowPos");
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
      SetWindowPos(hWnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOSENDCHANGING);
      auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
      MONITORINFO mi{};
      mi.cbSize = sizeof(MONITORINFO);
      GetMonitorInfoW(monitor, &mi);
      auto xres = mi.rcMonitor.right - mi.rcMonitor.left;
      auto yres = mi.rcMonitor.bottom - mi.rcMonitor.top;
      auto borders = getNonClientBorders(hWnd);
      w -= borders.width;
      h -= borders.height;
      window->postEvents(
        DisplayScaleEvent{scale, {(uint32_t)xres, (uint32_t)yres}},
        RawMovingEvent{{x, y}},
        RawResizeEvent{{w, h}}
      );
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

  bgfx::renderFrame((int32_t)(interval / 10'000ULL));

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
      bgfx::renderFrame((int32_t)(interval / 10'000ULL));
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
