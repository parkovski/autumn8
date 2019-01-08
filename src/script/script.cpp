#include "a8/script/script.h"

#include <iostream>

using namespace a8::script;

Script *Script::New() {
  return new Script;
}

Symbol::Symbol(Symbol *type, std::string_view name) noexcept
  : name{name}, data{}, type{type}
{}

Symbol::~Symbol() {
  // TODO: free Data if needed
}

Symbol &Script::addSymbol(Symbol *type, const char *name) {
  std::string_view name_sv{name};
  Symbol s{type, name_sv};
  return _symbols.emplace(name_sv, std::move(s)).first->second;
}

void do_insn(std::string_view t) {
  std::cout << t << "\n";
}

void Script::populateSymbols() {
  auto *insn = &addSymbol(nullptr, "insn");
  const char *x[] = {
    "new", "show", "ldi", "ldf", "lds", "ld", "set"
  };
  for (int i = 0; i < sizeof(x)/sizeof(x[0]); i++) {
    addSymbol(insn, x[i]).data.ptr(&do_insn);
  }
}

Script::Script()
  : _immexec{true}
{
  populateSymbols();
}

Script::~Script() {
}

bool Script::setexecmode(const char *mode) {
  using namespace std::literals::string_view_literals;
  if (mode == "immediate"sv) {
    _immexec = true;
  } else if (mode == "delayed"sv) {
    _immexec = false;
  } else {
    return false;
  }
  return true;
}

bool Script::run(const char *line) {
  auto const &line_str = _lines.emplace_back(line);

  std::vector<std::string_view> ts;
  int x0, x1 = 0;
  while ((x0 = x1) < line_str.length()) {
    ++x1;
    switch (line_str[x0]) {
      case '\0':
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        continue;
      default:
        break;
    }

    while (x1 < line_str.length()) {
      switch (line_str[x1]) {
        case '\0':
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          goto end_read_token;
        default:
          ++x1;
          continue;
      }
    }
    end_read_token:
    ts.emplace_back(std::string_view{line_str}.substr(x0, x1 - x0));
    std::cout << "> " << line_str.substr(x0, x1 - x0) << "\n";
  }

  if (ts.size()) {
    auto it = _symbols.find(ts[0]);
    if (it != _symbols.cend()) {
      //std::cout << "ok: " << s->first;
      if (it->second.type && it->second.type->name == "insn") {
        it->second.data.ptr<void (std::string_view)>()(it->first);
      }
    } else {
      std::cout << "not found: " << ts[0] << "\n";
    }
  }

  if (ts[0] == "q") {
    return false;
  }
  return true;
}
