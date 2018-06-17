#pragma once

#include "a8config.h"

#ifdef HAS_SPDLOG

# include "spdlog/spdlog.h"
namespace a8::internal {
  void initLogging(int argc, char *argv[]);
  void shutdownLogging();
}

namespace a8::internal {
  extern std::shared_ptr<spdlog::logger> g_logger;
}

template<typename... Args>
inline void logdebug(Args &&...args) {
  ::a8::internal::g_logger->debug(std::forward<Args>(args)...);
}

template<typename... Args>
inline void logtrace(Args &&...args) {
  ::a8::internal::g_logger->trace(std::forward<Args>(args)...);
}

template<typename... Args>
inline void loginfo(Args &&...args) {
  ::a8::internal::g_logger->info(std::forward<Args>(args)...);
}

template<typename... Args>
inline void logwarn(Args &&...args) {
  ::a8::internal::g_logger->warn(std::forward<Args>(args)...);
}

template<typename... Args>
inline void logerror(Args &&...args) {
  ::a8::internal::g_logger->error(std::forward<Args>(args)...);
}

template<typename... Args>
inline void logcritical(Args &&...args) {
  ::a8::internal::g_logger->critical(std::forward<Args>(args)...);
}

#else // HAS_SPDLOG

namespace a8::internal {
  inline void initLogging(int, char *[]);
  inline void shutdownLogging();
}

template<typename... Args>
inline void logdebug(Args &&...) { }

template<typename... Args>
inline void logtrace(Args &&...) { }

template<typename... Args>
inline void loginfo(Args &&...) { }

template<typename... Args>
inline void logwarn(Args &&...) { }

template<typename... Args>
inline void logerror(Args &&...) { }

template<typename... Args>
inline void logcritical(Args &&...) { }

#endif // HAS_SPDLOG

#ifdef _MSC_VER
# define A8_DLLEXPORT __declspec(dllexport)
#else
# define A8_DLLEXPORT
#endif

extern "C" int a8main(int argc, char *argv[]);
