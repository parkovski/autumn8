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
  static inline void debug(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::debug, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void trace(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::trace, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void info(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::info, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void warn(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::warn, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void error(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::err, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void critical(spdlog::format_string_t<Args...> f, Args &&...args) {
    if (g_logger) {
      g_logger->log(::spdlog::level::critical, f, std::forward<Args>(args)...);
    }
  }

  template<typename... Args>
  static inline void print(fmt::format_string<Args...> f, Args &&...args) {
    fmt::print(f, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static inline void eprint(fmt::format_string<Args...> f, Args &&...args) {
    fmt::print(stderr, f, std::forward<Args>(args)...);
  }
};

} // namespace a8
