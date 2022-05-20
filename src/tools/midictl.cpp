#include "a8/a8.h"
#include "a8/midi/access.h"
#include "a8/midi/port.h"

#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace a8;
using namespace a8::midi;

int a8main(int argc, char *argv[]) {
  std::shared_ptr<Access> access;
  if (argc == 2) {
    auto backend = parseBackend(argv[1]);
    if (backend == Backend::Null) {
      A8_ERROR("Invalid backend {}", argv[1]);
      return 1;
    }
    access = Access::request(backend).get();
  } else {
    access = Access::request().get();
  }
  if (!access) {
    A8_CRITICAL("MIDI access denied.");
    return 1;
  }

  {
    std::cout << "Inputs:\n";
    auto [inputLock, inputs] = access->inputsRead();
    for (auto &i : inputs) {
      auto const &port = i.second;
      std::cout << port->id() << " - " << port->name() << "\n";
    }
  }

  {
    std::cout << "\nOutputs:\n";
    auto [outputLock, outputs] = access->outputsRead();
    for (auto &o : outputs) {
      auto const &port = o.second;
      std::cout << port->id() << " - " << port->name() << "\n";
    }
  }

  std::cout << "\nOpen? ";
  std::string line;
  getline(std::cin, line);
  if (!line.empty()) {
    auto [inputLock, inputs] = access->inputsRead();
    auto it = inputs.find(line);
    if (it == inputs.end()) {
      std::cout << "Not found.\n";
    } else {
      auto p = it->second;
      inputLock.release();
      std::cout << "Open " << p->id() << " - " << p->name()
                << "\nPress enter when done.\n";

      p->open();
      p->onMIDIMessage(
        [](Input *port, uint32_t time, const Message &message) {
          std::cout << "Port " << port->name() << "(" << time << "): "
            << message << "\n";
        });

      getline(std::cin, line);

      p->close();
    }
  }

  return 0;
}
