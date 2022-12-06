#include "browser.h"

using namespace a8::app::browser;

Browser::Browser()
  : _curdir{std::filesystem::current_path()}
{}

const std::filesystem::path &Browser::curdir() const noexcept {
  return _curdir;
}

bool Browser::go_to(std::filesystem::path path) {
  std::error_code noone_cares;
  if (!std::filesystem::is_directory(path, noone_cares)) {
    return false;
  }
  _curdir = path;
  return true;
}

Directory Browser::list() const {
  Directory dir;
  std::error_code err;
  std::filesystem::directory_iterator di(curdir(), err);
  if (err) {
    return dir;
  }
  std::filesystem::directory_iterator end;
  dir.name = curdir().string();

  while (di != end) {
    FileKind kind = FileKind::Unknown;
    if (di->is_directory(err)) {
      kind = FileKind::Directory;
    } else if (di->is_symlink(err)) {
      kind = FileKind::Symlink;
    } else if (di->is_fifo(err)) {
      kind = FileKind::Fifo;
    } else if (di->is_socket(err)) {
      kind = FileKind::Socket;
    } else if (di->is_regular_file(err)) {
      kind = FileKind::File;
    }
    if (err) {
      continue;
    }
    dir.entries.emplace_back(DirEntry{kind, di->path().filename().string()});

    di.increment(err);
  }
  return dir;
}
