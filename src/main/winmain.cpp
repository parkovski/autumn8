#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"
#include "a8/util/utfconv.h"

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
  for (int i = 0; i < argc; ++i) {
    temp = to_utf8(argv16[i]);
    argv8[i] = reinterpret_cast<char *>(temp.length() + 1);
    if (i > 0) {
      argv8[i] += reinterpret_cast<size_t>(argv8[i - 1]);
    }
    buffer.append(temp).push_back('\0');

    if (i > 0 && temp == "--console") {
      g_platformInit.wantsConsole = true;
    }
  }
  LocalFree(argv16);
  argv16 = nullptr;
  for (int i = argc - 1; i > 0; --i) {
    argv8[i] = buffer.data() + reinterpret_cast<size_t>(argv8[i - 1]);
  }
  argv8[0] = buffer.data();

  g_platformInit.init(hInstance, nCmdShow);

  Log log{argc, argv8.get()};

  return a8main(argc, argv8.get());
}
