#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/window/window.h"
#include "a8/window/view.h"

using namespace a8::win;

namespace {
  static RawSize toSize(LPRECT rect) {
    return {(rect->right - rect->left), (rect->bottom - rect->top)};
  }

  static RawSize getNonClientBorders(HWND hWnd) {
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(hWnd, &wi);
    return toSize(&wi.rcWindow) - toSize(&wi.rcClient);
  }
}

LRESULT CALLBACK
a8::plat::Windows::mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  auto *window = reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

  switch (msg) {
  case WM_NCCREATE: {
    A8_TRACE("WM_NCCREATE");
    auto *createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
    window = reinterpret_cast<Window *>(createStruct->lpCreateParams);
    window->_nativeHandle = hWnd;
    window->_eventThread = std::thread{[window]() {
      auto result = window->processEvents();
      A8_DEBUG("Event thread returned {}.", result);
    }};
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
  }
    return true;

  case WM_CREATE: {
    A8_TRACE("WM_CREATE");
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
    A8_TRACE("WM_CLOSE");
    window->postEvent(SystemEvent::Close);
    return 0;

  case WM_USER:
    switch (wParam) {
      case A8M_ASYNCDESTROY:
        A8_TRACE("A8M_ASYNCDESTROY");
        DestroyWindow(hWnd);
        break;

      case A8M_EXECONMSGTHREAD: {
        A8_TRACE("A8M_EXECONMSGTHREAD");
        auto f = *(void (**)(void *data))lParam;
        void *data = ((void **)lParam)[1];
        f(data);
        break;
      }
    }
    return 0;

  case WM_DESTROY: {
    A8_TRACE("WM_DESTROY");
    // TODO: OMG
    if (window->_view) {
      delete window->_view;
      window->_view = nullptr;
    }
    window->postEvents(SystemEvent::Destroy, EventLoopQuitEvent{});
    return 0;
  }

  case WM_NCDESTROY: {
    A8_TRACE("WM_NCDESTROY");
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
    //A8_DEBUG("WM_SIZE; wParam = 0x{:X}", wParam);
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
    //A8_TRACE("WM_SIZING {}", side);

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
    A8_TRACE("WM_ENTERSIZEMOVE");
    break;

  case WM_EXITSIZEMOVE:
    A8_TRACE("WM_EXITSIZEMOVE");
    break;

  /*case WM_WINDOWPOSCHANGING: {
    auto pos = reinterpret_cast<LPWINDOWPOS>(lParam);
    const RawPoint point{pos->x, pos->y};
    const RawSize borderSize = getNonClientBorders(hWnd);
    RawSize size{pos->cx, pos->cy};
    if (size.width > 0 && size.height > 0) {
      size -= borderSize;
    }
    A8_TRACE("WM_WINDOWPOSCHANGING: {}, {} : {}x{}, flags=0x{:X}",
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
    A8_TRACE("WM_WINDOWPOSCHANGED: {}, {} : {}x{}; flags=0x{:X}",
             pos->x, pos->y, pos->cx, pos->cy, pos->flags);
    switch (pos->flags & (SWP_NOMOVE | SWP_NOSIZE)) {
    case 0:
      window->postEvents(RawResizeEvent{size}, RawMovingEvent{point});
      break;
    case SWP_NOMOVE:
      //A8_TRACE("WindowPos: {}, {}", pos->cx, pos->cy);
      window->postEvents(RawResizeEvent{size});
      break;
    case SWP_NOSIZE:
      //A8_TRACE("WindowPos: {}x{}", pos->x, pos->y);
      window->postEvent(RawMovingEvent{point});
      break;
    default:
      //A8_TRACE("WindowPos");
      break;
    }
  }
    return 0;*/

  case WM_SETFOCUS:
    A8_TRACE("WM_SETFOCUS");
    window->postEvent(SystemEvent::Focus);
    break;

  case WM_KILLFOCUS:
    A8_TRACE("WM_KILLFOCUS");
    window->postEvent(SystemEvent::Unfocus);
    break;

  case WM_DPICHANGED: {
    window->_scale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
    A8_TRACE("WM_DPICHANGED: {}", window->_scale);
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
