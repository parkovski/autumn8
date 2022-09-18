#ifndef MAPISH_LEX_H_
#define MAPISH_LEX_H_

#include <vector>
#include <string_view>

namespace mapish {

enum {
  print_text = 0,
  print_hex = 1,
  print_none = 2,
};

#define KEYWORDS(K) \
  K(list) \
  K(in) \
  K(out) \
  \
  K(channel) \
  K(note) \
  K(on) \
  K(off) \
  K(aftertouch) \
  K(polyphonic) \
  K(program) \
  K(pitchbend) \
  K(quarterframe) \
  K(song) \
  K(position) \
  K(select) \
  K(tunerequest) \
  K(timingclock) \
  K(start) \
  K(continue) \
  K(stop) \
  K(activesensing) \
  K(reset) \
  K(sysex) \

#define TOKENS(T) \
  T(invalid, "invalid character", hex) \
  T(eof, "end of file", none) \
  T(eol, "end of line", none) \
  T(semi, "semicolon", text) \
  T(word, "word", text) \
  T(int, "integer literal", text) \
  T(percent, "percentage", text) \
  T(notename, "note name", text) \
  T(channel, "MIDI channel", text) \

enum {
#define TK(Name, Desc, Print) tk_##Name,
  TOKENS(TK)
#undef TK
  _tk_count,
};

enum {
#define KW(Name) kw_##Name,
  KEYWORDS(KW)
#undef KW
  _kw_count,
};

struct Token {
  std::string_view text;
  unsigned line;
  unsigned column;
  unsigned line_to;
  unsigned column_to;
  unsigned kind;
  union {
    unsigned kw;
    int64_t i;
    double f;
  } value;

  bool is_keyword() const { return value.kw < _kw_count; }
};

std::string_view token_name(unsigned tk);
int token_print_kind(unsigned tk);
std::string_view keyword_name(unsigned kw);

void lex(std::vector<Token> &tokens, std::string_view text);

} // namespace mapish

#endif /* MAPISH_LEX_H_ */
