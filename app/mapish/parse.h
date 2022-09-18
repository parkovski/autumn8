#ifndef MAPISH_PARSE_H_
#define MAPISH_PARSE_H_

#include "lex.h"
#include "a8/midi/message.h"
#include <optional>

namespace mapish {

enum class ListKind {
  Inputs,
  Outputs,
};

struct Parser {
  explicit Parser(const std::vector<Token> &tokens) noexcept
    : _tokens{&tokens}
    , _index{0}
  {}

  bool parseSeparator();
  std::optional<uint8_t> parseNoteName();
  std::optional<ListKind> parseListKind();
  std::optional<a8::midi::Message> parseMidiMessage();

  // There is always an EOF token.
  bool is_end() const noexcept { return _index == _tokens->size() - 1; }

private:
  void error(std::string_view what);

  const Token &current() noexcept { return (*_tokens)[_index]; }
  bool next() noexcept {
    if (is_end()) {
      return false;
    }
    ++_index;
    return true;
  }

  const std::vector<Token> *_tokens;
  size_t _index;
};

} // namespace mapish

#endif /* MAPISH_PARSE_H_ */
