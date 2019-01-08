#ifndef A8_SCRIPT_TOKEN_H_
#define A8_SCRIPT_TOKEN_H_

namespace a8::script {

struct Token {
  class Document *doc;
  unsigned docfrom;
  unsigned docto;
  struct Symbol *attr;
};

} // namespace a8::script

#endif /* A8_SCRIPT_TOKEN_H_ */

