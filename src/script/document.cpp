#include "a8/script/document.h"

#include <iostream>

using namespace a8::script;

IDocument::~IDocument() {}

void IDocument::onAppend(unsigned split) {
  if (_onAppend) {
    _onAppend(*this, split);
  }
}

void StdinDocument::appendLine(std::string line) {
  _indexes.emplace_back(Index{(unsigned)_lines.size(), 0});
  _lines.emplace_back(std::move(line));
  onAppend(_indexes.size() - 1);
}

void StdinDocument::run() {
  std::string line;
  while (std::cin.good()) {
    std::getline(std::cin, line);
    appendLine(line);
  }
}

