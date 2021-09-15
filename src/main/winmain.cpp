#include "a8/a8.h"
#include "a8/platform/Windows/platform.h"
#include "a8/util/utfconv.h"

#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <memory>

#include <wrl.h>
#include <wil/com.h>
#include "../contrib/webview2/build/native/include/WebView2.h"

//#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "mincore.lib")

using a8::Log;
using namespace a8::platform::windows;
using a8::to_utf8;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  int argc;
  wchar_t **argv16 = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::string buffer{};
  std::string temp;
  auto argv8{std::make_unique<char *[]>(argc)};
  bool usingConsole = false;
  for (int i = 0; i < argc; ++i) {
    temp = to_utf8(argv16[i]);
    argv8[i] = reinterpret_cast<char *>(temp.length() + 1);
    if (i > 0) {
      argv8[i] += reinterpret_cast<size_t>(argv8[i - 1]);
    }
    buffer.append(temp).push_back('\0');

    if (i > 0 && temp == "--console" && !usingConsole) {
      usingConsole = true;
      if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
      }
    }
  }
  LocalFree(argv16);
  argv16 = nullptr;
  for (int i = argc - 1; i > 0; --i) {
    argv8[i] = buffer.data() + reinterpret_cast<size_t>(argv8[i - 1]);
  }
  argv8[0] = buffer.data();

  Log log{argc, argv8.get()};

  setThreadName(L"Message thread");
  auto dpiAwareness = enableThreadDpiAwareness();
  if (usingConsole) {
    Log::trace("DPI Awareness: {}.", dpiAwareness);
  }

  WNDCLASSEXW cls{};
  cls.cbSize = sizeof(WNDCLASSEXW);
  cls.style = CS_HREDRAW | CS_VREDRAW;
  cls.lpfnWndProc = mainWndProc;
  cls.hInstance = hInstance;
  cls.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  cls.hCursor = LoadCursor(nullptr, IDC_ARROW);
  cls.lpszClassName = A8WindowClass;
  cls.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

  if (!RegisterClassExW(&cls)) {
    Log::critical("RegisterClassExW failed.");
    return 1;
  }

  g_platformInit.hInstance = hInstance;
  g_platformInit.nCmdShow = nCmdShow;

  return a8main(argc, argv8.get());
}
