#include "a8/a8.h"
#include "a8/midi/midi.h"

#include <iostream>
#include <string>
#include <memory>
#include <mutex>

using namespace a8::midi;

int main(int argc, char *argv[]) {
  uwpMidiInitWinRT();
  std::string midiInterface;
  std::cout << "midi interface (uwp, winmm): ";
  std::cin >> midiInterface;
  MidiManager *pmm;
  if (midiInterface == "uwp") {
    pmm = newUwpMidiManager();
  } else if (midiInterface == "winmm") {
    pmm = newWinmmMidiManager();
  } else {
    std::cout << "Unknown interface type!\n";
    return 1;
  }
  std::unique_ptr<MidiManager> mm{pmm};
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
