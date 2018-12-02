#include "a8/a8.h"
#include "a8/platform/Windows/platform.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/ostream.h>

#include <Windows.h>
#include <shellapi.h>
#include <string_view>
#include <optional>
#include <string>
#include <memory>

#pragma comment(lib, "mincore.lib")

using namespace a8::platform::windows;

// std::shared_ptr<spdlog::logger> a8::internal::g_logger;

void a8::internal::initLogging(int argc, char *argv[]) {
  // g_logger = spdlog::stdout_color_mt("autumn8");
  // g_logger->set_level(spdlog::level::trace);
}

void a8::internal::shutdownLogging() {
  // Avoid VC++ deadlock bug
  // spdlog::drop_all();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  int argc;
  auto args = GetCommandLineW();
  wchar_t **argv16 = CommandLineToArgvW(args, &argc);
  std::string buffer{};
  std::string temp;
  auto argv8 = std::make_unique<char *[]>(argc);
  for (int i = 0; i < argc; ++i) {
    temp = to_utf8(argv16[i]);
    argv8[i] = (char *)(temp.length() + 1);
    if (i > 0) {
      argv8[i] += (size_t)argv8[i - 1];
    }
    buffer.append(temp).push_back('\0');

    if (i > 0 && temp == "--console") {
      AllocConsole();
    }
  }
  for (int i = argc - 1; i > 0; --i) {
    argv8[i] = buffer.data() + (size_t)(argv8[i - 1]);
  }
  argv8[0] = buffer.data();

  a8::internal::initLogging(argc, argv8.get());

  logtrace("WinMain: command line: {}", buffer);

  setThreadName(L"Message thread");
  logtrace("Message thread: {}", enableThreadDpiAwareness());

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
    return 1;
  }

  g_platformInit.hInstance = hInstance;
  g_platformInit.nCmdShow = nCmdShow;

  auto exitCode = a8main(argc, argv8.get());
  a8::internal::shutdownLogging();
  return exitCode;
}
