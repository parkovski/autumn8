#include "a8/a8.h"
#include "a8/midi.h"

#include <iostream>
#include <string>
#include <memory>
#include <mutex>

int main(int argc, char *argv[]) {
  a8::uwpMidiInitWinRT();
  std::cout << "monitoring midi devices:";
  std::unique_ptr<a8::MidiManager> mm{a8::newUwpMidiManager()};
  std::mutex dawg;
  mm->watchDevices([&](int id, bool connected) {
    std::lock_guard<std::mutex> jim(dawg);
    auto name = mm->deviceName(id);
    std::cout << "device " << (connected ? "connected" : "disconnected") <<
      ": " << name << "\n";
  });
  std::getline(std::cin, std::string());
  mm->stopWatchingDevices();
  return 0;
}
