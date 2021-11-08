#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"
#include "a8/util/utfconv.h"

#include <shellapi.h>
#include <string>
#include <memory>

#pragma comment(lib, "mincore.lib")

using a8::Log;
using namespace a8::platform::Windows;
using a8::to_utf8;

extern "C"
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  int argc;
  wchar_t **argv16 = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::string buffer{};
  auto argv8{std::make_unique<char *[]>(argc)};
  // In this first loop, all the arguments are converted to UTF-8 and appended
  // to buffer. Argv8 stores the offset of the next argument, which is the
  // sum of all the previous offsets plus the length of the current argument.
  for (int i = 0; i < argc; ++i) {
    std::wstring_view argw{argv16[i]};
    auto argsize = to_utf8(buffer, argw);
    buffer.push_back('\0');
    argv8[i] = reinterpret_cast<char *>(argsize + 1);
    if (i > 0) {
      argv8[i] += reinterpret_cast<size_t>(argv8[i - 1]);
    }

    if (i > 0 && argw == L"--console") {
      g_platformState.wantsConsole = true;
    }
  }
  LocalFree(argv16);
  argv16 = nullptr;
  // Now that the buffer pointer is fixed, we convert all the offsets stored
  // in argv8 to actual pointers to buffer.
  for (int i = argc - 1; i > 0; --i) {
    argv8[i] = buffer.data() + reinterpret_cast<size_t>(argv8[i - 1]);
  }
  argv8[0] = buffer.data();

  g_platformState.init(hInstance, nCmdShow);

  Log log{argc, argv8.get()};

  return a8main(argc, argv8.get());
}
