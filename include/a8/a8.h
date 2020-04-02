#ifndef A8_A8_H_
#define A8_A8_H_

#include "a8config.h"
#include "compiler.h"
#include "util/inline.h"

#include <memory>
#include <utility>
#include <fmt/format.h>
#include <fmt/ostream.h>
#ifdef _WIN32
# define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#endif
#include <spdlog/spdlog.h>

namespace a8 {

class Log {
  static std::shared_ptr<spdlog::logger> g_logger;

public:
  explicit Log(int argc, char *argv[]) noexcept;
  ~Log();

  template<typename... Args>
  static inline void debug(Args &&...args) {
    g_logger->log(::spdlog::level::debug, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void trace(Args &&...args) {
    g_logger->log(::spdlog::level::trace, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void info(Args &&...args) {
    g_logger->log(::spdlog::level::info, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void warn(Args &&...args) {
    g_logger->log(::spdlog::level::warn, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void error(Args &&...args) {
    g_logger->log(::spdlog::level::err, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void critical(Args &&...args) {
    g_logger->log(::spdlog::level::critical, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void print(Args &&...args) {
    fmt::print(std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void eprint(Args &&...args) {
    fmt::print(stderr, std::forward<Args>(args)...);
  }
};

} // namespace a8

extern "C" int a8main(int argc, char *argv[]);

#endif /* A8_A8_H_ */
