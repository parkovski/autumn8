#include "a8/a8.h"
#include "a8/midi.h"

#include <iostream>
#include <string>
#include <memory>
#include <mutex>

int main(int argc, char *argv[]) {
  a8::uwpMidiInitWinRT();
  std::string midiInterface;
  std::cout << "midi interface (uwp, winmm): ";
  std::cin >> midiInterface;
  a8::MidiManager *pmm;
  if (midiInterface == "uwp") {
    pmm = a8::newUwpMidiManager();
  } else if (midiInterface == "winmm") {
    pmm = a8::newWinmmMidiManager();
  } else {
    std::cout << "Unknown interface type!\n";
    return 1;
  }
  std::unique_ptr<a8::MidiManager> mm{pmm};
  std::cout << "monitoring midi devices:\n";
  std::mutex dawg;
  mm->watchDevices([&](int id, bool connected) {
    std::lock_guard<std::mutex> jim(dawg);
    auto name = mm->deviceName(id);
    std::cout << "device " << (connected ? "connected" : "disconnected") <<
      ": " << name << "\n";
  });
  while (midiInterface != "q") {
    std::cout << "> ";
    std::getline(std::cin, midiInterface);
  }
  mm->stopWatchingDevices();
  return 0;
}
