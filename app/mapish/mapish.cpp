#include "a8/a8.h"
#include "a8/midi/access.h"
#include "a8/midi/port.h"

#include "parse.h"

#include <string>
#include <iostream>

using namespace a8;
using namespace mapish;

struct Mapish {
  Mapish();

  void listInputs();
  void listOutputs();
private:
  std::shared_ptr<midi::Access> _access;
};

Mapish::Mapish() {
  _access = midi::Access::request().get();
}

void Mapish::listInputs() {
  auto [inputLock, inputs] = _access->inputsRead();
  for (auto &i : inputs) {
    auto const &port = i.second;
    A8_PRINT("{} - {}\n", port->id(), port->name());
  }
}

void Mapish::listOutputs() {
  auto [lock, outputs] = _access->outputsRead();
  for (auto &o : outputs) {
    auto const &port = o.second;
    A8_PRINT("{} - {}\n", port->id(), port->name());
  }
}

void run(Mapish &m, const std::vector<Token> tokens) {
  if (tokens[0].text == "list") {
    if (tokens.size() != 3) {
      A8_EPRINT("expected: list [inputs|outputs]\n");
      return;
    }
    if (tokens[1].text == "inputs") {
      m.listInputs();
    } else if (tokens[1].text == "outputs") {
      m.listOutputs();
    } else {
      A8_EPRINT("can't list that\n");
    }
  } else {
    A8_EPRINT("😡\n");
  }
}

int a8main(int argc, char *argv[]) {
  Mapish m;
  std::string line;
  std::vector<Token> tokens;

  while (true) {
    tokens.clear();
    std::cout << "mapish: ";
    getline(std::cin, line);
    if (std::cin.eof()) {
      break;
    }
    lex(tokens, line);
    Parser parser{tokens};
    auto omsg = parser.parseMidiMessage();
    if (omsg.has_value()) {
      A8_PRINT("> {}\n", omsg.value());
    }
    // for (auto const &t : tokens) {
    //   A8_PRINT("> {} {}\n", tknames[t.kind], t.text);
    // }
    //run(m, tokens);
  }
  return 0;
}
