#ifndef A8_TOOLS_BROWSER_MODEL_H_
#define A8_TOOLS_BROWSER_MODEL_H_

#include <nlohmann/json.hpp>

namespace a8::tools::browser {

struct DirEntry {
  std::string name;
  bool isDir;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirEntry, name, isDir)
};

struct DirList {
  std::string name;
  std::vector<DirEntry> entries;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirList, name, entries)
};

struct NavState {
  bool canGoBack;
  bool canGoForward;
  bool canGoUp;

  /*explicit NavState(const Browser &browser) noexcept
    : canGoBack{browser.can_go_back()},
      canGoForward{browser.can_go_forward()},
      canGoUp{browser.can_go_up()}
  {}*/

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(NavState, canGoBack, canGoForward, canGoUp)
};

} // namespace a8::tools::browser

#endif /* A8_TOOLS_BROWSER_MODEL_H_ */
