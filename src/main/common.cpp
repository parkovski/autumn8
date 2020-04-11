#include "a8/a8.h"

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static void getTermCaps(bool &vt, bool &ttyout, bool &ttyerr) {
  auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  auto hStdErr = GetStdHandle(STD_ERROR_HANDLE);
  ttyout = GetFileType(hStdOut) == FILE_TYPE_CHAR;
  ttyerr = GetFileType(hStdErr) == FILE_TYPE_CHAR;
  bool consoleVtSupported = false;
  if (ttyout) {
    DWORD mode;
    GetConsoleMode(hStdOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(hStdOut, mode)) {
      consoleVtSupported = true;
    }
  } else if (ttyerr) {
    DWORD mode;
    GetConsoleMode(hStdErr, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(hStdErr, mode)) {
      consoleVtSupported = true;
    }
  }
}

#else // _WIN32

#include <unistd.h>

static void getTermCaps(bool &vt, bool &ttyout, bool &ttyerr) {
  vt = true;
  ttyout = isatty(STDOUT_FILENO);
  ttyerr = isatty(STDERR_FILENO);
}

#endif // _WIN32

using a8::Log;

std::shared_ptr<spdlog::logger> Log::g_logger;

Log::Log(int argc, char *argv[]) {
  bool vt, ttyout, ttyerr;
  getTermCaps(vt, ttyout, ttyerr);

  std::string pattern;
  spdlog::level::level_enum level =
#ifdef NDEBUG
    spdlog::level::warn
#else
    spdlog::level::trace
#endif
    ;

  for (int i = 0; i < argc; ++i) {
    if (argv[i][0] == '-' && argv[i][1] == 'V') {
      switch (argv[i][2]) {
        case '0':
          level = spdlog::level::off;
          break;
        case '1':
        case 'c':
          level = spdlog::level::critical;
          break;
        case '2':
        case 'e':
          level = spdlog::level::err;
          break;
        case '3':
        case 'w':
          level = spdlog::level::warn;
          break;
        case '4':
        case 'i':
          level = spdlog::level::info;
          break;
        case '\0':
        case '5':
        case 'd':
          level = spdlog::level::debug;
          break;
        case '6':
        case 't':
          level = spdlog::level::trace;
          break;
      }
    } else if (std::string_view{argv[i]} == "--log-format" && i + 1 < argc) {
      pattern = argv[++i];
      if (pattern.find("%v") == std::string::npos) {
        pattern += " %v";
      }
    }
  }

  if (pattern.empty()) {
    if (ttyerr && vt) {
      pattern = "\033[90m[%T.%e] T#%-5t\033[m %^%5l:%$ %v";
    } else {
      pattern = "[%Y-%m-%d %T.%e] T#%-5t %5l: %v";
    }
  }

  spdlog::set_pattern(std::move(pattern));
  spdlog::set_level(level);
  if (ttyerr) {
    g_logger = spdlog::stderr_color_mt("autumn8");
  } else {
    g_logger = spdlog::stderr_logger_mt("autumn8");
  }
  g_logger->trace("Hello :)");
}

Log::~Log() {
  spdlog::drop_all();
}
