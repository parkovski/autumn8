#ifndef A8_MIDI_BACKEND_H_
#define A8_MIDI_BACKEND_H_

#include <iosfwd>
#include <string_view>

#include <fmt/ostream.h>

namespace a8::midi {

#define A8_MIDI_BACKENDS(M) \
    M(UWP)                  \
    M(Winmm)                \
    M(CoreMIDI)             \
    M(AMidi)                \
    M(AndroidMediaMidi)     \
    M(AlsaSeq)              \
    M(AlsaRaw)              \
    M(PipeWire)             \
    M(WebMIDI)              \
    M(Jack)

enum class Backend {
  Null,
#define A8_BACKEND(BE) BE,
  A8_MIDI_BACKENDS(A8_BACKEND)
#undef A8_BACKEND
  _count,
};

/**
 * Tells whether the platform supports the backend `be`. Does not necessarily
 * mean access will succeed, only that an implementation is available.
 */
bool supportsBackend(Backend be) noexcept;

/**
 * Get the platform's default backend.
 */
Backend defaultBackend() noexcept;

// Places a string representation of `be` into the stream `os`.
std::ostream &operator<<(std::ostream &os, Backend be);

// Parses a case insensitive string corresponding to a member of `Backend`.
// If none matches, returns `Backend::Null`.
Backend parseBackend(std::string_view name);

} // namespace a8::midi

namespace fmt {
  template<> struct formatter<a8::midi::Backend> : ostream_formatter {};
}

#endif /* A8_MIDI_BACKEND_H_ */
