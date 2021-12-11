#include "a8/a8.h"
#include "a8/midi/midi.h"

#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace a8;
using namespace a8::midi;

int a8main(int argc, char *argv[]) {
  std::unique_ptr<PortManager> pm;
  if (argc == 2) {
    auto backend = parseBackend(argv[1]);
    if (backend == Backend::Null) {
      Log::error("Invalid backend {}", argv[1]);
      return 1;
    }
    pm = PortManager::create(backend);
  } else {
    pm = PortManager::create();
  }
  if (!pm) {
    Log::critical("Port manager creation failed");
  }

  //std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::string sel;
  std::cout << "press enter to exit.\n";
  std::getline(std::cin, sel);
  if (!sel.empty()) {
    std::stringstream ss;
    ss << sel;
    int port, client;
    ss >> port >> client;
    Log::info("doing things {} {}", port, client);
    port = (client << 16) | port;
    auto it = pm->find(port);
    if (it != pm->end()) {
      auto *port = it->asInputPort();
      if (!port) {
        Log::debug("port is not input port");
      } else {
        port->open().wait();
        std::cout << "press enter to close port.\n";
        std::getline(std::cin, sel);
        port->close().wait();
      }
    }
  }

  return 0;
}
