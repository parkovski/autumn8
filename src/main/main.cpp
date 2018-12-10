#include "a8/a8.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_colors_sinks.h>

#include <memory>

std::shared_ptr<spdlog::logger> a8::internal::g_logger;

void a8::internal::initLogging(int argc, char *argv[]) {
  g_logger = spdlog::stdout_color_mt("autumn8");
  g_logger->set_level(spdlog::level::trace);
}

void a8::internal::shutdownLogging() {
  spdlog::drop_all();
}

#if defined(_WIN32)

#include "a8/util/utfconv.h"

// Windows entry point.
int wmain(int argc, wchar_t *argv16[]) {
  // Convert UTF-16 args to UTF-8.
  // RAII array of arg pointers.
  std::unique_ptr<char *[]> _argv8{argc};
  // UTF-8 argv.
  char *argv[] = _argv8.get();
  // Single buffer to hold all arguments, null separated.
  std::string _buffer{};
  // Temp conversion string.
  std::string _temp;
  for (int i = 0; i < argc; ++i) {
    _temp = a8::util::utfconv::to_utf8(argv16[i]);
    // Set each pointer to its end offset in `_buffer` (offset of next arg).
    // This offset is the length of the current string plus the previous
    // argument's offset.
    _argv8[i] = reinterpret_cast<char *>(temp.length() + 1);
    if (i > 0) {
      _argv8[i] += reinterpret_cast<size_t>(argv8[i - 1]);
    }
    // Append the converted string to the single buffer.
    _buffer.append(_temp).push_back('\0');
  }
  // Convert offsets to actual pointers into the buffer.
  for (int i = argc - 1; i > 0; --i) {
    _argv8[i] = _buffer.data() + reinterpret_cast<size_t>(argv8[i - 1]);
  }
  // First argument is at buffer offset 0.
  _argv8[0] = _buffer.data();

#else // defined(_WIN32)

// Non-Windows entry point.
int main(int argc, char *argv[]) {

#endif // defined(_WIN32)

  a8::internal::initLogging(argc, argv);
  auto exitCode = a8main(argc, argv);
  a8::internal::shutdownLogging();
  return exitCode;
}

