#include "a8/a8.h"
//#include "a8/util/log.h"

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using a8::Logger;

Logger Logger::s_main_logger;

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

void Logger::load_main_logger() {
  bool vt, ttyout, ttyerr;
  getTermCaps(vt, ttyout, ttyerr);

  spdlog::color_mode color_mode = spdlog::color_mode::automatic;
  std::string pattern;
  if (ttyerr && vt) {
    pattern = pattern_color;
    color_mode = spdlog::color_mode::always;
  } else {
    pattern = pattern_no_color;
    color_mode = spdlog::color_mode::never;
  }

  spdlog::level::level_enum level =
#ifdef NDEBUG
    spdlog::level::warn
#else
    spdlog::level::trace
#endif
    ;

  spdlog::set_pattern(std::move(pattern));
  spdlog::set_level(level);

  s_main_logger._logger =
    spdlog::stderr_color_mt(std::move(s_main_logger._name));
  A8_INFO("Hello from A8 😀");
}

void Logger::load_from_command_line(int argc, char *argv[]) {
  if (_logger) {
    return;
  }

  bool vt, ttyout, ttyerr;
  getTermCaps(vt, ttyout, ttyerr);

  spdlog::color_mode color_mode = spdlog::color_mode::automatic;
  std::string pattern;
  if (ttyerr && vt) {
    pattern = pattern_color;
    color_mode = spdlog::color_mode::always;
  } else {
    pattern = pattern_no_color;
    color_mode = spdlog::color_mode::never;
  }

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
        case '-':
        case 'o':
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
        case '5':
        case 'd':
          level = spdlog::level::debug;
          break;
        case '6':
        case 't':
          level = spdlog::level::trace;
          break;
        case '\0': {
          if (i + 1 == argc) {
#ifdef NDEBUG
            level = spdlog::level::info;
#else
            level = spdlog::level::trace;
#endif
            break;
          }

          std::string_view next{argv[i + 1]};
          if (next == "off") {
            level = spdlog::level::off;
          } else if (next == "critical") {
            level = spdlog::level::critical;
          } else if (next == "error") {
            level = spdlog::level::err;
          } else if (next == "warn" || next == "warning") {
            level = spdlog::level::warn;
          } else if (next == "info") {
            level = spdlog::level::info;
          } else if (next == "debug") {
            level = spdlog::level::debug;
          } else if (next == "trace") {
            level = spdlog::level::trace;
          } else {
#ifdef NDEBUG
            level = spdlog::level::info;
#else
            level = spdlog::level::trace;
#endif
            continue;
          }

          ++i;
          break;
        }
      }
    } else if (std::string_view{argv[i]} == "--log-format" && i + 1 < argc) {
      pattern = argv[++i];
      if (pattern == "no-color") {
        pattern = pattern_no_color;
        color_mode = spdlog::color_mode::never;
      } else if (pattern == "color") {
        pattern = pattern_color;
        color_mode = spdlog::color_mode::always;
      } else if (pattern == "no-color-explicit") {
        pattern = pattern_no_color_explicit;
        color_mode = spdlog::color_mode::never;
      } else if (pattern == "color-explicit") {
        pattern = pattern_color_explicit;
        color_mode = spdlog::color_mode::always;
      } else if (pattern.find("%v") == std::string::npos) {
        pattern += " %v";
      }
    }
  }

  spdlog::set_pattern(std::move(pattern));
  spdlog::set_level(level);
  _logger = spdlog::stderr_color_mt(std::move(_name), color_mode);
  A8_INFO("Hello from A8 😀");
}