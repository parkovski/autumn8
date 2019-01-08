#ifndef A8_SCRIPT_DOCUMENT_H_
#define A8_SCRIPT_DOCUMENT_H_

#include "script.h"

#include <functional>
#include <vector>
#include <string>
#include <string_view>

namespace a8::script {

class StdinDocument : public IDocument {
public:
  explicit StdinDocument() noexcept {}
  StdinDocument(const StdinDocument &) = delete;
  StdinDocument &operator=(const StdinDocument &) = delete;
  StdinDocument(StdinDocument &&) = default;
  StdinDocument &operator=(StdinDocument &&) = default;

  std::string_view name() const override {
    return "stdin";
  }

  void appendLine(std::string line);
  void run();

private:
  struct Index {
    unsigned line;
    unsigned column;
  };
  std::vector<std::string> _lines;
  std::vector<Index> _indexes;
};

} // namespace a8::script

#endif /* A8_SCRIPT_DOCUMENT_H_ */

