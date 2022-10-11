#ifndef A8_APP_BROWSER_BROWSER_H_
#define A8_APP_BROWSER_BROWSER_H_

#include <vector>
#include <filesystem>

namespace a8::app::browser {

class Browser {
  std::vector<std::filesystem::path> _stack;
  size_t _stackIndex;

public:
  using const_iterator = std::vector<std::filesystem::path>::const_iterator;
  using const_reverse_iterator
    = std::vector<std::filesystem::path>::const_reverse_iterator;

  explicit Browser();

  const std::filesystem::path &curdir() const noexcept;

  std::pair<const_reverse_iterator, const_reverse_iterator>
  get_back_range() const noexcept;

  std::pair<const_iterator, const_iterator>
  get_forward_range() const noexcept;

  bool can_go_back(size_t count = 1) const noexcept;

  bool can_go_forward(size_t count = 1) const noexcept;

  bool go_back(size_t count = 1) noexcept;

  bool go_forward(size_t count = 1) noexcept;

  /// \param overwrite Replace the current history entry instead of appending.
  bool go_to(std::filesystem::path path, bool overwrite = false);

  bool can_go_up() const noexcept;

  bool go_up();

  //DirList list() const;
};

} // namespace a8::app::browser

#endif /* A8_APP_BROWSER_BROWSER_H_ */
