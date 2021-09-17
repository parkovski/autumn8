#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <spdlog/spdlog.h>
#if defined(_WIN32) && !defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT)
# error "Windows needs SPDLOG_WCHAR_TO_UTF8_SUPPORT"
#endif

namespace a8 {

class Log {
  static std::shared_ptr<spdlog::logger> g_logger;

public:
  static constexpr const char *pattern_no_color = "[%T.%e #%-5t] %5l: %v";
  static constexpr const char *pattern_no_color_explicit = "[%Y-%m-%d %T.%e #%-5t] %5l: %v";
  static constexpr const char *pattern_color = "\033[90m[%T.%e #%-5t]\033[m %^%5l:%$ %v";
  static constexpr const char *pattern_color_explicit = "\033[90m[%Y-%m-%d %T.%e #%-5t]\033[m %^%5l:%$ %v";

  explicit Log(std::shared_ptr<spdlog::logger> logger) {
    g_logger = std::move(logger);
  }

  explicit Log(int argc, char *argv[]);

  ~Log() {
    spdlog::drop_all();
  }

  template<typename... Args>
  static inline void debug(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::debug, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void trace(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::trace, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void info(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::info, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void warn(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::warn, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void error(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::err, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void critical(Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::critical, std::forward<Args>(args)...);
    }
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
