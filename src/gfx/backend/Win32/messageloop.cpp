#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/gfx/window.h"
#include "a8/gfx/views/view.h"
#include "a8/util/utfconv.h"

#include "fmt/ostream.h"

#include <string>
#include <future>
#include <atomic>
#include <ostream>

#define MY_HWND static_cast<HWND>(this->_nativeHandle)
#define HWND_OF(w) static_cast<HWND>((w).nativeHandle())

using a8::Log;
using namespace a8::platform::windows;
using namespace a8::gfx;

namespace {
  // To not lose precision by storing coordinates as int.
  constexpr int InitialCoordMultiplier = 60;
}

Window::~Window() {}

bool Window::open(std::string_view title, void *parent) noexcept {
  DWORD style = parent
    ? WS_CHILD | WS_VISIBLE
    : WS_OVERLAPPEDWINDOW;

  auto u16title = to_utf16(title);
  _nativeHandle = CreateWindowEx(
    WS_EX_ACCEPTFILES,
    A8WindowClass,
    u16title.c_str(),
    style,
    _pos.x,
    _pos.y,
    _size.width,
    _size.height,
    (HWND)parent,
    nullptr,
    g_platformInit.hInstance,
    this
  );

  if (!_nativeHandle) {
    return false;
  }

  if (!parent) {
    ShowWindow(MY_HWND, g_platformInit.nCmdShow);
    UpdateWindow(MY_HWND);
  }
  return true;
}

void Window::close() noexcept {
  PostMessage(MY_HWND, WM_USER, 0, 0);
}

void Window::show(bool show) noexcept {
  if (show) {
    ShowWindow(MY_HWND, SW_SHOW);
    UpdateWindow(MY_HWND);
  } else {
    ShowWindow(MY_HWND, SW_HIDE);
  }
}

void Window::zoom(float newZoom) {
  _zoom = newZoom;
}

void Window::size(Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  if (_size == rawNewSize) {
    return;
  }
  _size = rawNewSize;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(MY_HWND, nullptr, 0, 0, _size.width, _size.height,
               SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOZORDER
                | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

void Window::position(Point newPosition) {
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_pos == rawNewPosition) {
    return;
  }
  _pos = rawNewPosition;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(MY_HWND, nullptr, _pos.x, _pos.y, 0, 0,
               SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER
                | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
}

void Window::resize(Point newPosition, Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_size == rawNewSize && _pos == rawNewPosition) {
    return;
  }
  _size = rawNewSize;
  _pos = rawNewPosition;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(MY_HWND, nullptr, _pos.x, _pos.y, _size.width, _size.height,
               SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOOWNERZORDER
                | SWP_NOACTIVATE);
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
  auto *window = reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

  switch (msg) {
  case WM_NCCREATE: {
    Log::trace("WM_NCCREATE");
    auto *createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
    window = reinterpret_cast<Window *>(createStruct->lpCreateParams);
    window->_nativeHandle = hWnd;
    window->_eventThread = std::thread{[window]() {
      auto result = window->processEvents();
      Log::debug("Event thread returned {}.", result);
    }};
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
  }
    return true;

  case WM_CREATE: {
    Log::trace("WM_CREATE");
    auto *createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
    float scale = GetDpiForWindow(hWnd) / (float)USER_DEFAULT_SCREEN_DPI;
    window->_scale = scale;
    window->_pos = RawPoint{
      int(createStruct->x * scale),
      int(createStruct->y * scale)
    };
    window->_size = RawSize{
      int(createStruct->cx * scale),
      int(createStruct->cy * scale)
    };
    window->postEvent(SystemEvent::Initialize);
    SetWindowPos(hWnd, nullptr, window->_pos.x, window->_pos.y,
                 window->_size.width, window->_size.height,
                 SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING
                  | SWP_NOACTIVATE);
    // TODO: Probably do this after Initialize event.
    if (window->_view) {
      window->_view->init(window);
    }
  }
    return 0;

  case WM_CLOSE:
    Log::trace("WM_CLOSE");
    window->postEvent(SystemEvent::Close);
    return 0;

  case WM_USER:
    Log::trace("WM_USER (A8 Async Destroy)");
    DestroyWindow(hWnd);
    return 0;

  case WM_DESTROY: {
    Log::trace("WM_DESTROY");
    // TODO: OMG
    if (window->_view) {
      delete window->_view;
      window->_view = nullptr;
    }
    window->postEvents(SystemEvent::Destroy, EventLoopQuitEvent{});
    return 0;
  }

  case WM_NCDESTROY: {
    Log::trace("WM_NCDESTROY");
    if (window->_quitOnClose) {
      PostQuitMessage(0);
    }
    window->_eventThread.join();
    delete window;
    return 0;
  }

  case WM_MOUSEMOVE: {
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    // TODO: Are any buttons down?
    window->postEvent(
      RawPointerEvent(
        {x, y}, 1, PointerKind::Mouse, PointerAction::Moved, false, 0, 0
      )
    );
  }
    return 0;

  case WM_SIZE: {
    RawSize size{(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
    Log::debug("WM_SIZE; wParam = 0x{:X}", wParam);
    switch (wParam) {
    case SIZE_MINIMIZED:
      window->postEvent(SystemEvent::Minimize);
      break;
    case SIZE_MAXIMIZED:
      window->postEvent(SystemEvent::Maximize);
      break;
    // case SIZE_RESTORED:
    //   window->postEvent(SystemEvent::RestoreSize);
    //   break;
    default:
      break;
    }
    if (window->_view) {
      window->_view->resize({0, 0}, size);
    }
    window->postEvent(RawResizeEvent{size});
  }
    return 0;

  case WM_SIZING: {
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
    Log::trace("WM_SIZING {}", side);

    RECT *rc = reinterpret_cast<RECT *>(lParam);
    auto size = toSize(rc) - getNonClientBorders(hWnd);
    window->postEvent(RawSizingEvent{size, side});
  }
    return true;

  case WM_MOVING: {
    auto rc = reinterpret_cast<LPRECT>(lParam);
    window->postEvent(RawMovingEvent{{rc->left, rc->top}});
  }
    return true;

  case WM_ENTERSIZEMOVE:
    Log::trace("WM_ENTERSIZEMOVE");
    break;

  case WM_EXITSIZEMOVE:
    Log::trace("WM_EXITSIZEMOVE");
    break;

  /*case WM_WINDOWPOSCHANGING: {
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

  case WM_WINDOWPOSCHANGED: {
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
    return 0;*/

  case WM_SETFOCUS:
    Log::trace("WM_SETFOCUS");
    window->postEvent(SystemEvent::Focus);
    break;

  case WM_KILLFOCUS:
    Log::trace("WM_KILLFOCUS");
    window->postEvent(SystemEvent::Unfocus);
    break;

  case WM_DPICHANGED: {
    window->_scale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
    auto rect = reinterpret_cast<LPRECT>(lParam);
    auto x = rect->left;
    auto y = rect->top;
    auto w = rect->right - x;
    auto h = rect->bottom - y;
    SetWindowPos(hWnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOSENDCHANGING);
    auto monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{};
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &mi);
    auto xres = mi.rcMonitor.right - mi.rcMonitor.left;
    auto yres = mi.rcMonitor.bottom - mi.rcMonitor.top;
    window->postEvent(
      DisplayScaleEvent{window->_scale, {(uint32_t)xres, (uint32_t)yres}}
    );
  }
    return 0;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

int a8::gfx::runMessageLoop() {
  if (g_platformInit.dwMainThreadId != GetCurrentThreadId()) {
    Log::warn("Expected runMessageLoop() on thread {}",
              g_platformInit.dwMainThreadId);
  }

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

int a8::gfx::runConstantFPSMessageLoop(void (*callback)(), int fps) {
  if (g_platformInit.dwMainThreadId != GetCurrentThreadId()) {
    Log::warn("Expected runConstantFPSMessageLoop() on thread {}",
              g_platformInit.dwMainThreadId);
  }

  // Units are 100ns intervals. There are 10 million of these in a second.
  const ULONGLONG interval = 10'000'000 / fps;
  ULONGLONG currentTime;
  MSG msg;

  while (true) {
    QueryInterruptTime(&currentTime);
    ULONGLONG nextTime = currentTime + interval;
    if (currentTime < nextTime) {
      // 10,000 "intervals" per millisecond.
      auto ms = (DWORD)((nextTime - currentTime) / 10'000ULL);
      if (MsgWaitForMultipleObjects(0, nullptr, false, ms, QS_ALLINPUT) != WAIT_TIMEOUT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          if (msg.message == WM_QUIT) {
            break;
          }
        }
        continue;
      }
    }

    // Either currentTime >= nextTime or MsgWaitForMultipleObjects returned
    // WAIT_TIMEOUT, either way it is time to process per-FPS events.
    callback();
  }

  return (int)msg.wParam;
}

void a8::gfx::postQuitMessage(int exitCode) {
  PostQuitMessage(exitCode);
}
