#include "a8/a8.h"

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

static void getTermCaps(bool &vt, bool &ttyout, bool &ttyerr);

using a8::Log;

std::shared_ptr<spdlog::logger> Log::g_logger;

a8::Log::Log(int argc, char *argv[]) noexcept {
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
      pattern = "\033[90m[%T.%e] T#%5t\033[m %^%l:%$ %v";
    } else {
      pattern = "[%Y-%m-%d %T.%e] T#%5t %l: %v";
    }
  }

  spdlog::set_pattern(pattern);
  spdlog::set_level(level);
  if (ttyerr) {
    g_logger = spdlog::stderr_color_mt("autumn8");
  } else {
    g_logger = spdlog::stderr_logger_mt("autumn8");
  }
  g_logger->trace("Hello :)");
}

a8::Log::~Log() {
  spdlog::drop_all();
}

#if defined(_WIN32)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "a8/util/utfconv.h"

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

// Windows entry point.
int wmain(int argc, wchar_t *argv16[]) {
  // Convert UTF-16 args to UTF-8.
  // RAII array of arg pointers.
  auto _argv8{std::make_unique<char *[]>(argc)};
  // Single buffer to hold all arguments, null separated.
  std::string _buffer{};
  // Temp conversion string.
  std::string _temp;
  for (int i = 0; i < argc; ++i) {
    _temp = a8::util::utfconv::to_utf8(argv16[i]);
    // Set each pointer to its end offset in `_buffer` (offset of next arg).
    // This offset is the length of the current string plus the previous
    // argument's offset.
    _argv8[i] = reinterpret_cast<char *>(_temp.length() + 1);
    if (i > 0) {
      _argv8[i] += reinterpret_cast<size_t>(_argv8[i - 1]);
    }
    // Append the converted string to the single buffer.
    _buffer.append(_temp).push_back('\0');
  }
  // Convert offsets to actual pointers into the buffer.
  for (int i = argc - 1; i > 0; --i) {
    _argv8[i] = _buffer.data() + reinterpret_cast<size_t>(_argv8[i - 1]);
  }
  // First argument is at buffer offset 0.
  _argv8[0] = _buffer.data();
  // UTF-8 argv.
  char **argv = _argv8.get();

#else // defined(_WIN32)

#include <unistd.h>

static void getTermCaps(bool &vt, bool &ttyout, bool &ttyerr) {
  vt = true;
  ttyout = isatty(STDOUT_FILENO);
  ttyerr = isatty(STDERR_FILENO);
}

// Non-Windows entry point.
int main(int argc, char **argv) {

#endif // defined(_WIN32)

  a8::Log log{argc, argv};
  return a8main(argc, argv);
}

