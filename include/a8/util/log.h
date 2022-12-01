#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#if __has_include(<source_location>)
# include <source_location>
#endif

#ifndef __cpp_lib_source_location
# if __has_include(<experimental/source_location>)
#   include <experimental/source_location>
    namespace std {
      using experimental::source_location;
    }
# else
#   error "Missing std::source_location"
# endif
#endif

#ifndef A8_LOGGER
# ifdef A8_MODULE_NAME
#  define A8_WANTS_MODULE_LOGGER
#  define A8_LOGGER ::a8::g_module_logger
# else
#  define A8_LOGGER ::a8::Logger::s_main_logger
# endif
#endif

#define A8_TRACE(...)    A8_LOGGER.trace(::std::source_location::current(), __VA_ARGS__)
#define A8_DEBUG(...)    A8_LOGGER.debug(::std::source_location::current(), __VA_ARGS__)
#define A8_INFO(...)     A8_LOGGER.info(::std::source_location::current(), __VA_ARGS__)
#define A8_WARN(...)     A8_LOGGER.warn(::std::source_location::current(), __VA_ARGS__)
#define A8_ERROR(...)    A8_LOGGER.error(::std::source_location::current(), __VA_ARGS__)
#define A8_CRITICAL(...) A8_LOGGER.critical(::std::source_location::current(), __VA_ARGS__)

#define A8_PRINT(...) ::fmt::print(__VA_ARGS__)
#define A8_EPRINT(...) ::fmt::print(stderr, __VA_ARGS__)

namespace a8 {

class Logger {
public:
  static Logger s_main_logger;

  static constexpr const char
    *pattern_no_color = "[%T.%e #%-5t] %n: %5l: %v",
    *pattern_no_color_explicit = "[%Y-%m-%d %T.%e #%-5t] %n: %5l: %v",
    *pattern_color = "\033[90m[%T.%e #%-5t] %n:\033[m %^%5l:%$ %v",
    *pattern_color_explicit =
      "\033[90m[%Y-%m-%d %T.%e #%-5t] %n:\033[m %^%5l:%$ %v";

private:
  std::shared_ptr<spdlog::logger> _logger;
  std::string _name;

  constexpr static spdlog::source_loc
  spdlog_loc(const std::source_location &loc) noexcept {
    return spdlog::source_loc(loc.file_name(), static_cast<int>(loc.line()),
                              loc.function_name());
  }

  static void load_main_logger();

public:
  constexpr Logger() noexcept
    : _logger{},
      _name{"Autumn8"}
  {}

  explicit Logger(std::shared_ptr<spdlog::logger> logger) noexcept
    : _logger{std::move(logger)},
      _name{"Autumn8"}
  {}

  constexpr explicit Logger(std::string name) noexcept
    : _logger{},
      _name{std::move(name)}
  {}

  void load_from_command_line(int argc, char **argv);

  void set(std::shared_ptr<spdlog::logger> logger) noexcept {
    _logger = std::move(logger);
  }

  const std::shared_ptr<spdlog::logger> &get() noexcept {
    if (!_logger) [[unlikely]] {
      if (!(_logger = spdlog::get(_name))) {
        if (&s_main_logger == this) {
          load_main_logger();
        } else {
          _logger = s_main_logger._logger->clone(std::move(_name));
        }
      }
    }
    return _logger;
  }

  template<class... Args>
  void log(std::source_location loc,
           spdlog::level::level_enum level,
           spdlog::format_string_t<Args...> format,
           Args &&...args) {
    get()->log(spdlog_loc(loc), level, format, std::forward<Args>(args)...);
  }

  template<class... Args>
  void log(spdlog::level::level_enum level,
           spdlog::format_string_t<Args...> format,
           Args &&...args) {
    get()->log(level, format, std::forward<Args>(args)...);
  }

  template<class... Args>
  void log(std::source_location loc,
           spdlog::level::level_enum level,
           std::string_view msg) {
    get()->log(spdlog_loc(loc), level, msg);
  }

  template<class... Args>
  void log(spdlog::level::level_enum level,
           std::string_view msg) {
    get()->log(level, msg);
  }

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
  template<class... Args>
  void log(std::source_location loc,
           spdlog::level::level_enum level,
           spdlog::wformat_string_t<Args...> format,
           Args &&...args) {
    get()->log(spdlog_loc(loc), level, format, std::forward<Args>(args)...);
  }

  template<class... Args>
  void log(spdlog::level::level_enum level,
           spdlog::wformat_string_t<Args...> format,
           Args &&...args) {
    get()->log(level, format, std::forward<Args>(args)...);
  }

  template<class... Args>
  void log(std::source_location loc,
           spdlog::level::level_enum level,
           std::wstring_view msg) {
    get()->log(spdlog_loc(loc), level, msg);
  }

  template<class... Args>
  void log(spdlog::level::level_enum level,
           std::wstring_view msg) {
    get()->log(level, msg);
  }
#endif

#define A8_LOG_METHODS(Name, Level, FmtPrefix)                       \
  template<class... Args>                                            \
  void Name(std::source_location loc,                                \
            spdlog::FmtPrefix##format_string_t<Args...> format,      \
            Args &&...args) noexcept {                               \
    get()->log(spdlog_loc(loc), spdlog::level::Level, format,        \
               std::forward<Args>(args)...);                         \
  }                                                                  \
                                                                     \
  template<class... Args>                                            \
  void Name(spdlog::FmtPrefix##format_string_t<Args...> format,      \
            Args &&...args) noexcept {                               \
    get()->log(spdlog::level::Level, format,                         \
               std::forward<Args>(args)...);                         \
  }                                                                  \
                                                                     \
  void Name(std::source_location loc,                                \
            std::FmtPrefix##string_view msg) noexcept {              \
    get()->log(spdlog_loc(loc), spdlog::level::Level, msg);          \
  }                                                                  \
                                                                     \
  void Name(std::FmtPrefix##string_view msg) noexcept {              \
    get()->log(spdlog::level::Level, msg);                           \
  }

  A8_LOG_METHODS(trace, trace,)
  A8_LOG_METHODS(debug, debug,)
  A8_LOG_METHODS(info, info,)
  A8_LOG_METHODS(warn, warn,)
  A8_LOG_METHODS(error, err,)
  A8_LOG_METHODS(critical, critical,)

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
  A8_LOG_METHODS(trace, trace, w)
  A8_LOG_METHODS(debug, debug, w)
  A8_LOG_METHODS(info, info, w)
  A8_LOG_METHODS(warn, warn, w)
  A8_LOG_METHODS(error, err, w)
  A8_LOG_METHODS(critical, critical, w)
#endif

#undef A8_LOG_METHODS
};

#ifdef A8_WANTS_MODULE_LOGGER
namespace {
  Logger g_module_logger{std::string{A8_MODULE_NAME}};
}
#endif

} // namespace a8
