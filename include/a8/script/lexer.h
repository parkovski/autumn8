#ifndef A8_SCRIPT_LEXER_H_
#define A8_SCRIPT_LEXER_H_

#include "document.h"

#include <memory>

namespace a8::script {

struct Token {
  unsigned begin;
  unsigned end;
};

class Lexer {
public:
  void next(IDocument &doc, unsigned split);
};

} // namespace a8::script

#endif /* A8_SCRIPT_LEXER_H_ */

