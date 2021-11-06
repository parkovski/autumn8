#include "a8/a8.h"
#include "a8/midi/midi.h"

#include <iostream>
#include <cstdlib>
#include <thread>

using namespace a8;
using namespace a8::midi;

int a8main(int argc, char *argv[]) {
#if A8_IS_PLATFORM(Windows)
  Backend backend = Backend::Winmm;
  if (argc == 2) {
    std::string_view backend_arg{argv[1]};
    if (backend_arg == "winmm") {
      backend = Backend::Winmm;
    } else if (backend_arg == "null") {
      backend = Backend::Null;
    // } else if (backend_arg == "uwp") {
    //   backend = Backend::UWP;
    } else {
      Log::error("Invalid backend {}", backend_arg);
      return 1;
    }
  }
  auto inMan = PortManager<InputPort>::create(backend);
  auto outMan = PortManager<OutputPort>::create(backend);
#else
  auto inMan = PortManager<InputPort>::create();
  auto outMan = PortManager<OutputPort>::create();
#endif

  inMan->scan().wait();
  outMan->scan().wait();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::string sel;
  std::cout << "input device number: ";
  std::getline(std::cin, sel);
  int n = atoi(sel.c_str());
  if (auto it = inMan->get(n); it != inMan->end()) {
    auto inp = inMan->open(it).get();
    inp->sink([](auto const &m, auto ts) {
      Log::debug("in: size={}", m.size());
    });
    inp->start();
    std::cout << "Press enter to exit.\n";
    std::getline(std::cin, sel);
    Port::destroy(std::move(inp)).wait();
  }

  return 0;
}
