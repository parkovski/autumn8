#define A8_INCLUDE_PLATFORM
#include "a8/a8.h"

using namespace a8::platform;

#if defined(_WIN32)

#include "a8/util/utfconv.h"

#include <string>
#include <memory>

// Windows console entry point.
extern "C"
int wmain(int argc, wchar_t *argv16[]) {
  // Convert UTF-16 args to UTF-8.
  // RAII array of arg pointers.
  auto _argv8{std::make_unique<char *[]>(argc)};
  // Single buffer to hold all arguments, null separated.
  std::string _buffer{};
  for (int i = 0; i < argc; ++i) {
    // Append argv16[i] to buffer as UTF-8 and add a NUL terminator.
    auto argsize = a8::to_utf8(_buffer, argv16[i]);
    _buffer.push_back('\0');
    // Set each pointer to its end offset in `_buffer` (offset of next arg).
    // This offset is the length of the current string plus the previous
    // argument's offset.
    _argv8[i] = reinterpret_cast<char *>(argsize + 1);
    if (i > 0) {
      _argv8[i] += reinterpret_cast<size_t>(_argv8[i - 1]);
    }
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

// Non-Windows entry point.
int main(int argc, char **argv) {

#endif // defined(_WIN32)

  a8::Log log{argc, argv};
  g_platformState.init(argc, argv);
  return a8main(argc, argv);
}
