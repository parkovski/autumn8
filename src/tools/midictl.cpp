#include "a8/a8.h"
#include "a8/midi/midi.h"

#include <iostream>
#include <cstdlib>
#include <thread>

using namespace a8;
using namespace a8::midi;

int a8main(int argc, char *argv[]) {
  Backend backend;
  if (argc == 2) {
    std::string_view backend_arg{argv[1]};
    if (backend_arg == "winmm") {
      backend = Backend::Winmm;
    } else if (backend_arg == "uwp") {
      backend = Backend::UWP;
    } else {
      Log::error("Invalid backend {}", backend_arg);
      return 1;
    }
  } else {
    backend = Backend::Default;
  }

  auto inMan = PortManager<InputPort>::create(backend);
  auto outMan = PortManager<OutputPort>::create(backend);

  inMan->scan().wait();
  outMan->scan().wait();

  std::string sel;
  std::getline(std::cin, sel);
  int n = atoi(sel.c_str());
  if (auto it = inMan->get(n); it != inMan->end()) {
    auto inp = inMan->open(it).get();
    // Now what?
    std::getline(std::cin, sel);
  }

  return 0;
}
