#ifndef A8_SCRIPT_SCRIPT_H_
#define A8_SCRIPT_SCRIPT_H_

#include "../a8.h"
#include "symbol.h"

#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>

namespace a8::script {

class IDocument {
public:
  using AppendFn = void(IDocument &, unsigned);
  using EditFn = void(IDocument &, unsigned, unsigned);

  IDocument() = default;
  IDocument(const IDocument &) = default;
  IDocument &operator=(const IDocument &) = default;
  IDocument(IDocument &&) = default;
  IDocument &operator=(IDocument &&) = default;
  virtual ~IDocument() = 0;

  void onAppend(std::function<AppendFn> &&f) {
    _onAppend = std::move(f);
  }

  void onAppend(unsigned split);

  virtual std::string_view name() const = 0;

private:
  std::function<AppendFn> _onAppend;
};

class Script {
  std::vector<std::string> _lines;
  std::unordered_map<std::string_view, Symbol> _symbols;
  bool _immexec;

  Symbol &addSymbol(Symbol *type, const char *name);
  void populateSymbols();

public:
  A8_PUBLIC Script *New();

  A8_PUBLIC explicit Script();
  A8_PUBLIC ~Script();

  std::vector<std::string> &lines() { return _lines; }
  std::unordered_map<std::string_view, Symbol> &symbols() { return _symbols; }

  // immediate or delayed
  // returns was mode set?
  A8_PUBLIC bool setexecmode(const char *mode);
  A8_PUBLIC bool run(const char *line);
};

} // namespace a8::script

#endif /* A8_SCRIPT_SCRIPT_H_ */
