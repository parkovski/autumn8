#ifndef A8_A8_H_
#define A8_A8_H_

#include "a8config.h"
#include "compiler.h"

#include <memory>
#include <utility>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace a8 {

class Log {
  static std::shared_ptr<spdlog::logger> g_logger;

public:
  explicit Log(int argc, char *argv[]) noexcept;
  ~Log();

  template<typename Fmt, typename... Args>
  static inline void debug(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::debug, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void trace(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::trace, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void info(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::info, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void warn(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::warn, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void error(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::error, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void critical(Fmt &&fmt, Args &&...args) {
    g_logger->log(::spdlog::level::critical, std::forward<Fmt>(fmt),
                  std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void print(Fmt &&fmt, Args &&...args) {
    fmt::print(std::forward<Fmt>(fmt), std::forward<Args>(args)...);
  }

  template<typename Fmt, typename... Args>
  static inline void eprint(Fmt &&fmt, Args &&...args) {
    fmt::print(stderr, std::forward<Fmt>(fmt), std::forward<Args>(args)...);
  }
};

} // namespace a8

extern "C" int a8main(int argc, char *argv[]);

#endif /* A8_A8_H_ */

