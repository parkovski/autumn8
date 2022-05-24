#include "a8/a8.h"
#include "a8/midi/access.h"
#include "a8/midi/port.h"

#include <iostream>

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
    A8_PRINT("Inputs:\n");
    auto [inputLock, inputs] = access->inputsRead();
    for (auto &i : inputs) {
      auto const &port = i.second;
      A8_PRINT("{} - {}\n", port->id(), port->name());
    }
  }

  {
    A8_PRINT("\nOutputs:\n");
    auto [outputLock, outputs] = access->outputsRead();
    for (auto &o : outputs) {
      auto const &port = o.second;
      A8_PRINT("{} - {}\n", port->id(), port->name());
    }
  }

  A8_PRINT("\nOpen? ");
  std::string line;
  getline(std::cin, line);
  if (!line.empty()) {
    auto [inputLock, inputs] = access->inputsRead();
    auto it = inputs.find(line);
    if (it == inputs.end()) {
      A8_PRINT("Not found.\n");
    } else {
      auto p = it->second;
      inputLock.release();
      A8_PRINT("Open {} - {}.\nPress enter when done.\n", p->id(), p->name());

      p->open();
      p->onMIDIMessage(
        [](Input *port, uint32_t time, const Message &message) {
          A8_PRINT("Port {} (T+{}): {}\n", port->name(), time, message);
        });

      getline(std::cin, line);

      p->close();
    }
  }

  return 0;
}
