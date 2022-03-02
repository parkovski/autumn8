#ifndef A8_MIDI_ACCESS_H_
#define A8_MIDI_ACCESS_H_

#include "backend.h"

#include <future>
#include <memory>

namespace a8::midi {

struct Options {
  bool sysex = false;
};

class Access;

/**
 * Requests MIDI access using the first successful backend.
 */
std::future<std::unique_ptr<Access>>
requestAccess(Options options = {});

/**
 * Requests MIDI access using a specific backend.
 */
std::future<std::unique_ptr<Access>>
requestAccess(Backend be, Options options = {});

class Access {
protected:
  explicit Access();

public:
  void *inputs() const;
  void *outputs() const;
  void *onStateChange();
  bool sysexEnabled() const noexcept;
};

} // namespace a8::midi

#endif /* A8_MIDI_ACCESS_H_ */
