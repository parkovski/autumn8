#pragma once

#include "a8config.h"

#ifdef HAS_SPDLOG

#include <memory>
#include <utility>
#include "fmt/format.h"
#include "spdlog/spdlog.h"

namespace a8::internal {
  void initLogging(int argc, char *argv[]);
  void shutdownLogging();

  extern std::shared_ptr<spdlog::logger> g_logger;
}

template<typename... Args>
void logdebug(Args &&...args) {
  using namespace ::a8::internal;
  auto &l = *g_logger;
  l.log(::spdlog::level::debug, std::forward<Args>(args)...);
}

template<typename... Args>
void logtrace(Args &&...args) {
  using namespace ::a8::internal;
  auto &l = *g_logger;
  l.log(::spdlog::level::trace, std::forward<Args>(args)...);
}

template<typename... Args>
void loginfo(Args &&...args) {
  using namespace ::a8::internal;
  g_logger->log(::spdlog::level::info, std::forward<Args>(args)...);
}

template<typename... Args>
void logwarn(Args &&...args) {
  using namespace ::a8::internal;
  g_logger->log(::spdlog::level::warn, std::forward<Args>(args)...);
}

template<typename... Args>
void logerror(Args &&...args) {
  using namespace ::a8::internal;
  g_logger->log(::spdlog::level::error, std::forward<Args>(args)...);
}

template<typename... Args>
void logcritical(Args &&...args) {
  using namespace ::a8::internal;
  g_logger->log(::spdlog::level::critical, std::forward<Args>(args)...);
}

#else // HAS_SPDLOG

namespace a8::internal {
  void initLogging(int, char *[]);
  void shutdownLogging();
}

template<typename... Args>
void logdebug(const char *, Args &&...) { }

template<typename... Args>
void logtrace(const char *, Args &&...) { }

template<typename... Args>
void loginfo(const char *, Args &&...) { }

template<typename... Args>
void logwarn(const char *, Args &&...) { }

template<typename... Args>
void logerror(const char *, Args &&...) { }

template<typename... Args>
void logcritical(const char *, Args &&...) { }

#endif // HAS_SPDLOG

#ifdef _MSC_VER

# define A8_DLLEXPORT __declspec(dllexport)
#else
# define A8_DLLEXPORT
#endif

extern "C" int a8main(int argc, char *argv[]);
