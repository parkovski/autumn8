#include "a8/a8.h"
#include "a8/gfx/gfx.h"

#include <bx/thread.h>
#include <bx/timer.h>
#include <bx/spscqueue.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <Windows.h>
#include <shellapi.h>
#include <string_view>
#include <thread>
#include <optional>

#pragma comment(lib, "mincore.lib")

namespace a8::gfx {
  struct NativeState {
    LPWSTR args;
    HWND hWnd;
    bgfx::ViewId viewId;
    bool running;

    NativeState(LPWSTR args) noexcept : args(args), running(true) {}
  };

  void postQuitEvent(NativeState *state) {
    PostMessageW(state->hWnd, WM_QUIT, 0, 0);
  }
}

using namespace a8::gfx;

namespace {

template<typename F, typename... Args>
auto dynamicCall(const wchar_t *dll, const char *fname, Args&&... args)
  -> std::optional<decltype(std::declval<F>()(std::forward<Args>(args)...))>
{
  auto dllHandle = GetModuleHandleW(dll);
  if (dllHandle) {
    auto function = reinterpret_cast<F>(GetProcAddress(dllHandle, fname));
    if (function) {
      return function(std::forward<Args>(args)...);
    }
  }
  return std::nullopt;
}

#define DYNAMIC_CALL(dll, fn, ...) \
  dynamicCall<decltype(&fn)>(dll, #fn, __VA_ARGS__)

int32_t mainThread(Window &window) {
  auto *state = window.nativeState();
  int argc;
  wchar_t const *const *argv = CommandLineToArgvW(state->args, &argc);

  DYNAMIC_CALL(L"kernel32.dll", SetThreadDescription,
    GetCurrentThread(), L"Main event thread"
  );

  bgfx::Init init;
  init.deviceId = 0;
  init.resolution.width = 500;
  init.resolution.height = 500;
  init.resolution.reset = BGFX_RESET_VSYNC;
  init.type = bgfx::RendererType::Count;
  init.vendorId = BGFX_PCI_ID_NONE;
  if (!bgfx::init(init) || !window.nvgInit()) {
    return 1;
  }

  bgfx::setDebug(BGFX_DEBUG_TEXT);
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

  while (state->running) {
    bgfx::setViewRect(window.viewId(), 0, 0, 500, 500);
    window.render();
    // bgfx::setViewRect(0, 0, 0, 500, 500);
    // bgfx::touch(0);
    // bgfx::dbgTextClear();
    // bgfx::dbgTextPrintf(0, 1, 0x0f, "Mouse: (%d, %d)", state.mx, state.my);

    bgfx::frame();
  }

  window.nvgDestroy();
  bgfx::shutdown();

  return 0;
}

} // anonymous namespace

LRESULT CALLBACK mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  auto *window = reinterpret_cast<Window *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

  switch (msg) {
  case WM_CREATE:
    {
      auto *createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
      window = reinterpret_cast<Window *>(createStruct->lpCreateParams);
      SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    break;
  case WM_CLOSE:
    window->postEvent(SystemEvent::Close);
    return 0;
  case WM_DESTROY:
  case WM_NCDESTROY:
    return 0;
  case WM_MOUSEMOVE:
    window->postEvent(MouseEvent(LOWORD(lParam), HIWORD(lParam), 0));
    break;
  case WM_DPICHANGED:
    {
      auto rect = reinterpret_cast<LPRECT>(lParam);
      auto x = rect->left;
      auto y = rect->top;
      auto w = rect->right - x;
      auto h = rect->bottom - y;
      SetWindowPos(hWnd, nullptr, x, y, w, h, SWP_NOZORDER);
    }
    break;
  }

  return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR args, int nCmdShow) {
  if (std::wstring_view(args).find(L"--debug") != std::wstring_view::npos) {
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
      AllocConsole();
    }
  }

  DYNAMIC_CALL(L"kernel32.dll", SetThreadDescription,
    GetCurrentThread(), L"Message thread"
  );

  if (!DYNAMIC_CALL(
    L"user32.dll", SetThreadDpiAwarenessContext,
    DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
  )) {
    dynamicCall<HRESULT (WINAPI *)(int)>(
      L"shcore.dll", "SetProcessDpiAwareness",
      2 // PROCESS_PER_MONITOR_DPI_AWARE
    );
  }

  WNDCLASSEXW cls{};
  cls.cbSize = sizeof(WNDCLASSEXW);
  cls.style = CS_HREDRAW | CS_VREDRAW;
  cls.lpfnWndProc = mainWndProc;
  cls.hInstance = instance;
  cls.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  cls.hCursor = LoadCursor(nullptr, IDC_ARROW);
  cls.lpszClassName = L"Autumn8";
  cls.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

  if (!RegisterClassExW(&cls)) {
    return 1;
  }

  NativeState state{args};
  Window window{0, &state};

  HWND hWnd = CreateWindowExW(
    WS_EX_ACCEPTFILES,
    L"Autumn8",
    L"Autumn8",
    WS_OVERLAPPEDWINDOW,
    0, 0, 500, 500,
    nullptr,
    nullptr,
    instance,
    &window
  );

  if (!hWnd) {
    return 1;
  }

  state.hWnd = hWnd;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  bgfx::PlatformData platformData{};
  platformData.nwh = hWnd;
  bgfx::setPlatformData(platformData);

  std::thread thread(mainThread, std::ref(window));

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

  DestroyWindow(hWnd);

  return (int)msg.wParam;
}
