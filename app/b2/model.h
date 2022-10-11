#ifndef A8_APP_BROWSER_MODEL_H_
#define A8_APP_BROWSER_MODEL_H_

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <memory>

namespace a8::app::browser {

enum class FileKind {
  Unknown,
  File,
  Directory,
  SymLink,
  Fifo,
  Socket,
  Reparse,
};

NLOHMANN_JSON_SERIALIZE_ENUM(FileKind, {
  { FileKind::Unknown, nullptr },
  { FileKind::File, "file" },
  { FileKind::Directory, "directory" },
  { FileKind::SymLink, "symlink" },
  { FileKind::Fifo, "fifo" },
  { FileKind::Socket, "socket" },
  { FileKind::Reparse, "reparse" },
});

struct DirEntry {
  FileKind kind;
  std::string name;

  explicit DirEntry(FileKind kind = FileKind::Unknown) noexcept
    : kind{kind}
  {}

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirEntry, kind, name)
};

struct File : DirEntry {
  std::string mime_type;

  explicit File() noexcept
    : DirEntry(FileKind::File)
  {}
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

} // namespace a8::app::browser

#endif /* A8_APP_BROWSER_MODEL_H_ */
