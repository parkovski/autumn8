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
  Symlink,
  Fifo,
  Socket,
  Reparse,
};

NLOHMANN_JSON_SERIALIZE_ENUM(FileKind, {
  { FileKind::Unknown, nullptr },
  { FileKind::File, "file" },
  { FileKind::Directory, "directory" },
  { FileKind::Symlink, "symlink" },
  { FileKind::Fifo, "fifo" },
  { FileKind::Socket, "socket" },
  { FileKind::Reparse, "reparse" },
});

struct DirEntry {
  FileKind kind{FileKind::Unknown};
  std::string name{};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirEntry, kind, name)
};

struct File : DirEntry {
  File() : DirEntry{FileKind::File} {}

  std::string mimeType{};
  std::vector<char> data{};
};

struct Directory : DirEntry {
  Directory() : DirEntry{FileKind::Directory} {}

  std::vector<DirEntry> entries{};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Directory, kind, name, entries)
};

struct Symlink : DirEntry {
  Symlink() : DirEntry{FileKind::Symlink} {}

  std::string target{};
};

struct Reparse : DirEntry {
  Reparse() : DirEntry{FileKind::Reparse} {}

  std::string tag{};
  std::string wellKnownTag{};
};

} // namespace a8::app::browser

#endif /* A8_APP_BROWSER_MODEL_H_ */
