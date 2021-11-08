#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"
#include "a8/gfx/msgloop.h"

using a8::Log;
using namespace a8::platform::Windows;

int a8::gfx::runMessageLoop() {
  if (g_platformState.dwMainThreadId != GetCurrentThreadId()) {
    Log::warn("Expected runMessageLoop() on thread {}",
              g_platformState.dwMainThreadId);
  }

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

int a8::gfx::runConstantFPSMessageLoop(void (*callback)(), int fps) {
  if (g_platformState.dwMainThreadId != GetCurrentThreadId()) {
    Log::warn("Expected runConstantFPSMessageLoop() on thread {}",
              g_platformState.dwMainThreadId);
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
