#include "lex.h"
#include <cassert>
#include <unordered_map>

using namespace mapish;

std::string_view mapish::token_name(unsigned tk) {
  static const std::string_view names[] = {
#define TK(Name, Desc, Print) Desc,
    TOKENS(TK)
#undef TK
  };

  assert(tk < sizeof(names) / sizeof(names[0]));
  return names[tk];
}

int mapish::token_print_kind(unsigned tk) {
  static const int8_t kinds[] = {
#define TK(Name, Desc, Print) print_##Print,
    TOKENS(TK)
#undef TK
  };

  assert(tk < sizeof(kinds) / sizeof(kinds[0]));
  return kinds[tk];
}

std::string_view mapish::keyword_name(unsigned kw) {
  static const std::string_view names[] = {
#define KW(Name) #Name,
    KEYWORDS(KW)
#undef KW
  };

  assert(kw < sizeof(names) / sizeof(names[0]));
  return names[kw];
}

namespace {

struct Lexer {
  Lexer(std::string_view text)
    : text{text.data()}
    , pos{0}
    , len{text.size()}
    , line{1}
    , column{1}
  {
    if (len) {
      ch = text[0];
    } else {
      ch = 0;
    }

    for (unsigned i = 0; i < _kw_count; ++i) {
      keywords.emplace(keyword_name(i), i);
    }
  }

  bool nextch();
  bool read(Token &token);

  bool is_eof() const { return pos == len; }
  bool is_ws() const;
  bool is_word() const;

  void skip_ws();

  const char *text;
  size_t pos;
  size_t len;
  unsigned line;
  unsigned column;
  char ch;
  std::unordered_map<std::string_view, unsigned> keywords;
};

bool Lexer::nextch() {
  if (ch == '\n') {
    ++line;
    column = 1;
  } else {
    ++column;
  }

  if (pos + 1 < len) {
    ch = text[++pos];
    return true;
  } else if (pos + 1 == len) {
    ++pos;
  }

  ch = 0;
  return false;
}

bool Lexer::read(Token &token) {
  skip_ws();
  token.line = line;
  token.column = column;
  if (is_eof()) {
    token.text = "";
    token.line_to = line;
    token.column_to = column;
    token.kind = tk_eof;
    return false;
  }

  size_t start = pos;

  switch (ch) {
  case '\n':
    token.kind = tk_eol;
    break;
  case ';':
    token.kind = tk_semi;
    break;
  default:
    if (is_word()) {
      token.kind = tk_word;
      while (nextch() && is_word()) {
        continue;
      }
      if (auto it = keywords.find({text + start, pos - start});
          it != keywords.end()) {
        token.value.kw = it->second;
      } else {
        token.value.kw = _kw_count;
      }
    } else {
      token.kind = tk_invalid;
      nextch();
    }
  }

  token.text = {text + start, pos - start};
  token.line_to = line;
  token.column_to = column;
  return true;
}

bool Lexer::is_ws() const {
  switch (ch) {
  case ' ':
  case '\t':
  case '\r':
    return true;
  default:
    return false;
  }
}

bool Lexer::is_word() const {
  if (char lower = ch | 0x20; lower >= 'a' && lower <= 'z') {
    return true;
  }
  if (ch >= '0' && ch <= '9') {
    return true;
  }
  switch (ch) {
  case '~':
  case '!':
  case '@':
  case '#':
  case '%':
  case '-':
  case '+':
  case '=':
  case ':':
  case '.':
    return true;

  default:
    return false;
  }
}

void Lexer::skip_ws() {
  while (is_ws() && nextch()) {
    continue;
  }
}

} // anonymous namespace

void mapish::lex(std::vector<Token> &tokens, std::string_view text) {
  Token t;
  Lexer l{text};
  while (l.read(t)) {
    tokens.emplace_back(t);
  }
  // eof
  tokens.emplace_back(t);
}
