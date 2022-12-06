#ifndef A8_APP_BROWSER_BROWSER_H_
#define A8_APP_BROWSER_BROWSER_H_

#include "model.h"

#include <vector>
#include <filesystem>

namespace a8::app::browser {

class Browser {
  std::filesystem::path _curdir;

public:
  explicit Browser();

  const std::filesystem::path &curdir() const noexcept;

  bool go_to(std::filesystem::path path);

  Directory list() const;
};

} // namespace a8::app::browser

#endif /* A8_APP_BROWSER_BROWSER_H_ */
