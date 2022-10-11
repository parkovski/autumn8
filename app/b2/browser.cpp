#include "browser.h"

using namespace a8::app::browser;

Browser::Browser()
  : _stack{std::filesystem::current_path()},
    _stackIndex{0}
{}

const std::filesystem::path &
Browser::curdir() const noexcept {
  return _stack[_stackIndex];
}


auto Browser::get_back_range() const noexcept
  -> std::pair<const_reverse_iterator, const_reverse_iterator>
{
  return std::make_pair(
    const_reverse_iterator{_stack.begin() + _stackIndex}, _stack.rend());
}

auto Browser::get_forward_range() const noexcept
  -> std::pair<const_iterator, const_iterator>
{
  return std::make_pair(_stack.begin() + (_stackIndex + 1), _stack.end());
}

bool Browser::can_go_back(size_t count) const noexcept {
  return _stackIndex + 1 > count;
}

bool Browser::can_go_forward(size_t count) const noexcept {
  return _stackIndex + count < _stack.size();
}

bool Browser::go_back(size_t count) noexcept {
  if (!can_go_back(count)) {
    return false;
  }
  _stackIndex -= count;
  return true;
}

bool Browser::go_forward(size_t count) noexcept {
  if (!can_go_forward(count)) {
    return false;
  }
  _stackIndex += count;
  return true;
}

/// \param overwrite Replace the current history entry instead of appending.
bool Browser::go_to(std::filesystem::path path, bool overwrite) {
  std::error_code noone_cares;
  if (!std::filesystem::is_directory(path, noone_cares)) {
    return false;
  }
  if (overwrite) {
    _stack[_stackIndex] = std::move(path);
    return true;
  }
  if (can_go_forward()) {
    // Pushing a new path overwrites the forward history.
    _stack.erase(_stack.cbegin() + _stackIndex + 1, _stack.cend());
  }
  _stack.emplace_back(std::move(path));
  ++_stackIndex;
  return true;
}

bool Browser::can_go_up() const noexcept {
  return curdir().has_parent_path();
}

bool Browser::go_up() {
  if (!can_go_up()) {
    return false;
  }
  return go_to(curdir().parent_path());
}

#if 0
DirList Browser::list() const {
  DirList dir;
  std::error_code err;
  std::filesystem::directory_iterator di(curdir(), err);
  if (err) {
    return dir;
  }
  std::filesystem::directory_iterator end;
  dir.name = curdir().string();

  while (di != end) {
    bool isdir = di->is_directory(err);
    if (err) {
      continue;
    }
    dir.entries.emplace_back(DirEntry{di->path().filename().string(), isdir});

    di.increment(err);
  }
  return dir;
}
#endif
