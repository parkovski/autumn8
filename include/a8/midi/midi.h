#ifndef A8_MIDI_H_
#define A8_MIDI_H_

#include "a8/a8.h"
#include "control.h"
#include "message.h"

#include <string_view>
#include <future>
#include <memory>
#include <mutex>
#include <exception>
#include <iosfwd>
#include <unordered_map>
#include <array>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/container_hash/hash.hpp>

namespace a8::midi {

#define A8_MIDI_BACKENDS(M) \
    M(UWP)                  \
    M(Winmm)                \
    M(CoreMIDI)             \
    M(AMidi)                \
    M(AndroidMediaMidi)     \
    M(Alsa)                 \
    M(WebMIDI)              \
    M(Jack)

enum class Backend {
  Null,
#define BACKEND(BE) BE,
  A8_MIDI_BACKENDS(BACKEND)
#undef BACKEND
  _count,
};

constexpr const std::array ActiveBackends{
#if A8_IS_PLATFORM(Windows)
  Backend::UWP,
  Backend::Winmm,
#elif A8_IS_PLATFORM(macOS) || A8_IS_PLATFORM(iOS)
  Backend::CoreMIDI,
#elif A8_IS_PLATFORM(Android)
  Backend::AMidi,
  Backend::AndroidMediaMidi,
#elif A8_IS_PLATFORM(Linux)
  Backend::Alsa,
#elif A8_IS_PLATFORM(Emscripten)
  Backend::WebMIDI,
#endif

#if A8_HAS_JACK
  Backend::Jack,
#endif

  Backend::Null,
};

// Places a string representation of `be` into the stream `os`.
std::ostream &operator<<(std::ostream &os, Backend be);

// Parses a case insensitive string corresponding to a member of `Backend`.
// If none matches, returns `Backend::Null`.
Backend parseBackend(std::string_view name);

class InputPort;
class OutputPort;

// Common MIDI port base type.
class Port {
public:
  enum class Connection {
    Disconnected,
    Connected,
    Pending,
  };

  using timestamp_t = uint32_t;

protected:
  explicit Port(boost::uuids::uuid id, std::string name) noexcept;

  Port(const Port &) = delete;
  Port &operator=(const Port &) = delete;

  virtual ~Port() = 0;

public:
  // Unique ID of this port.
  const boost::uuids::uuid &id() const noexcept {
    return _id;
  }

  // Human-friendly name of this port.
  std::string_view name() const noexcept {
    return _name;
  }

  // WebMIDI calls this `state` and it has the string values "connected" and
  // "disconnected".
  virtual bool deviceConnected() const noexcept = 0;

  virtual Connection connection() const noexcept = 0;

  // TODO: onstatechange

  /// \returns A future which resolves to a pointer to this port when the
  ///          close operation completes.
  virtual std::future<Port *> close() = 0;

  /// WebMIDI uses a string enum `type` here with value "input".
  /// \returns This port casted to `InputPort` if it is in fact an `InputPort`,
  ///          otherwise null.
  virtual InputPort *asInputPort() noexcept;
  const InputPort *asInputPort() const noexcept {
    return const_cast<Port *>(this)->asInputPort();
  }

  /// WebMIDI uses a string enum `type` here with value "output".
  /// \returns This port casted to `OutputPort` if it is in fact an
  ///          `OutputPort`, otherwise null.
  virtual OutputPort *asOutputPort() noexcept;
  const OutputPort *asOutputPort() const noexcept {
    return const_cast<Port *>(this)->asOutputPort();
  }

private:
  boost::uuids::uuid _id;
  std::string _name;
};

class InputPort : public Port {
protected:
  using Port::Port;

public:
  using message_handler_type =
    std::function<void(InputPort *, const IMessage &, Port::timestamp_t)>;

  void onMessage(message_handler_type handler) noexcept {
    _handler = std::move(handler);
  }

  void onMessage() noexcept {
    _handler = &InputPort::nop;
  }

  /// \returns A future which resolves to a pointer to this port when the
  ///          open operation completes.
  virtual std::future<InputPort *> open() = 0;

  InputPort *asInputPort() noexcept override final;

protected:
  void emit(const IMessage &message, Port::timestamp_t timestamp) {
    _handler(this, message, timestamp);
  }

private:
  static void nop(InputPort *, const IMessage &, Port::timestamp_t);

  message_handler_type _handler = &InputPort::nop;
};

class OutputPort : public Port {
protected:
  using Port::Port;

public:
  virtual void send(const unsigned char *begin, const unsigned char *end,
                    Port::timestamp_t timestamp = 0) = 0;

  void send(const IMessage &message, Port::timestamp_t timestamp = 0) {
    send(message.data(), message.data() + message.size(), timestamp);
  }

  virtual std::future<OutputPort *> open() = 0;

  OutputPort *asOutputPort() noexcept override final;
};

class PortManager {
  template<Backend B>
  static std::unique_ptr<PortManager> createInternal();

protected:
  using map_t = std::unordered_map<boost::uuids::uuid, std::shared_ptr<Port>,
                                   boost::hash<boost::uuids::uuid>>;

  PortManager() = default;

  map_t &portMap() {
    return _ports;
  }

  boost::uuids::uuid nextId() {
    return _uuidGen();
  }

public:
  virtual ~PortManager() = 0;

  // Create a `PortManager` for the given backend.
  static std::unique_ptr<PortManager> create(Backend backend);

  // Create a `PortManager` for the first available backend for the current
  // platform.
  static std::unique_ptr<PortManager> create();

  std::unique_lock<std::recursive_mutex> lock() const {
    return std::unique_lock<std::recursive_mutex>{_mutex};
  }

  std::unique_lock<std::recursive_mutex> tryLock() const {
    return std::unique_lock<std::recursive_mutex>{_mutex, std::try_to_lock};
  }

  // \returns the current number of accessible ports. The size may change if a
  // locked iterator is not held.
  size_t size() const noexcept {
    auto lk = lock();
    return _ports.size();
  }

  /// \returns true if the port collection is currently empty. \see size().
  bool empty() const noexcept {
    return size() == 0;
  }

  // TODO: onstatechange

  template<class MapIter>
  struct iterator_base {
  protected:
    iterator_base(std::unique_lock<std::recursive_mutex> lock,
                  MapIter mapIterator) noexcept
      : _lock{std::move(lock)},
        _mapIterator{std::move(mapIterator)}
    {}

  public:
    iterator_base(const iterator_base &other) noexcept
      : _lock{*other._lock.mutex()},
        _mapIterator{other._mapIterator}
    {}

    iterator_base &operator=(const iterator_base &other) noexcept {
      _lock = std::unique_lock<std::recursive_mutex>{*other._lock.mutex()};
      _mapIterator = other._mapIterator;
      return *this;
    }

    iterator_base(iterator_base &&) noexcept = default;
    iterator_base &operator=(iterator_base &&) noexcept = default;

    bool operator==(const iterator_base &other) const noexcept {
      return _mapIterator == other._mapIterator;
    }

    bool operator!=(const iterator_base &other) const noexcept {
      return !(*this == other);
    }

    const Port *operator*() const noexcept {
      return _mapIterator->second.get();
    }

    const Port *operator->() const noexcept {
      return _mapIterator->second.get();
    }

  protected:
    Port *get() noexcept {
      return _mapIterator->second.get();
    }

    std::unique_lock<std::recursive_mutex> _lock;
    MapIter _mapIterator;
  };

  struct const_iterator : iterator_base<map_t::const_iterator> {
  private:
    friend class PortManager;

    const_iterator(std::unique_lock<std::recursive_mutex> lock,
                   map_t::const_iterator mapIterator) noexcept
      : iterator_base(std::move(lock), std::move(mapIterator))
    {}

  public:
    const_iterator(const const_iterator &other) noexcept
      : iterator_base(other)
    {}

    const_iterator &operator=(const const_iterator &other) noexcept {
      iterator_base::operator=(other);
      return *this;
    }

    const_iterator(const_iterator &&) noexcept = default;
    const_iterator &operator=(const_iterator &&) noexcept = default;

    using iterator_base::operator==;
    using iterator_base::operator!=;

    const_iterator &operator++() noexcept {
      ++_mapIterator;
      return *this;
    }

    const_iterator operator++(int) noexcept {
      const_iterator prev = *this;
      ++_mapIterator;
      return prev;
    }
  };

  struct iterator : iterator_base<map_t::iterator> {
  private:
    friend class PortManager;

    iterator(std::unique_lock<std::recursive_mutex> lock,
             map_t::iterator mapIterator) noexcept
      : iterator_base(std::move(lock), std::move(mapIterator))
    {}

  public:
    iterator(const iterator &other) noexcept
      : iterator_base(other)
    {}

    iterator &operator=(const iterator &other) noexcept {
      iterator_base::operator=(other);
      return *this;
    }

    iterator(iterator &&) noexcept = default;
    iterator &operator=(iterator &&) noexcept = default;

    using iterator_base::operator==;
    using iterator_base::operator!=;

    iterator &operator++() noexcept {
      ++_mapIterator;
      return *this;
    }

    iterator operator++(int) noexcept {
      iterator prev = *this;
      ++_mapIterator;
      return prev;
    }

    // TODO: shared_ptr
    Port *operator*() noexcept {
      return get();
    }

    Port *operator->() noexcept {
      return get();
    }
  };

  const_iterator cbegin() const noexcept {
    return {lock(), _ports.cbegin()};
  }

  const_iterator begin() const noexcept { return cbegin(); }

  // Acquires a lock.
  iterator begin() noexcept {
    return {lock(), _ports.begin()};
  }

  const_iterator cend() const noexcept {
    return {lock(), _ports.end()};
  }

  const_iterator end() const noexcept { return cend(); }

  // Acquires a lock.
  iterator end() noexcept {
    return {lock(), _ports.end()};
  }

  // const_reverse_iterator crbegin() const noexcept {}
  // const_reverse_iterator rbegin() const noexcept { return crbegin(); }
  // reverse_iterator rbegin() noexcept {}

  // const_reverse_iterator crend() const noexcept {}
  // const_reverse_iterator rend() const noexcept { return crend(); }
  // reverse_iterator rend() noexcept {}

  iterator find(const boost::uuids::uuid &id) noexcept {
    auto lk = lock();
    auto it = _ports.find(id);
    return {std::move(lk), std::move(it)};
  }

  const_iterator find(const boost::uuids::uuid &id) const noexcept {
    auto lk = lock();
    auto it = _ports.find(id);
    return {std::move(lk), std::move(it)};
  }

  iterator findInput(const boost::uuids::uuid &id) noexcept {
    auto it = find(id);
    auto endit = end();
    if (it != endit && it->asInputPort()) {
      return it;
    }
    return endit;
  }

  const_iterator findInput(const boost::uuids::uuid &id) const noexcept {
    auto it = find(id);
    auto endit = end();
    if (it != endit && it->asInputPort()) {
      return it;
    }
    return endit;
  }

  iterator findOutput(const boost::uuids::uuid &id) noexcept {
    auto it = find(id);
    auto endit = end();
    if (it != endit && it->asOutputPort()) {
      return it;
    }
    return endit;
  }

  const_iterator findOutput(const boost::uuids::uuid &id) const noexcept {
    auto it = find(id);
    auto endit = end();
    if (it != endit && it->asOutputPort()) {
      return it;
    }
    return endit;
  }

  // TODO: Change to shared_ptr.
  Port *get(const boost::uuids::uuid &id) noexcept {
    auto it = find(id);
    if (it == end()) {
      return nullptr;
    }
    return *it;
  }

  const Port *get(const boost::uuids::uuid &id) const noexcept {
    auto it = find(id);
    if (it == end()) {
      return nullptr;
    }
    return *it;
  }

  InputPort *getInput(const boost::uuids::uuid &id) noexcept {
    if (auto port = get(id)) {
      return port->asInputPort();
    }
    return nullptr;
  }

  const InputPort *getInput(const boost::uuids::uuid &id) const noexcept {
    return const_cast<PortManager *>(this)->getInput(id);
  }

  OutputPort *getOutput(const boost::uuids::uuid &id) noexcept {
    if (auto port = get(id)) {
      return port->asOutputPort();
    }
    return nullptr;
  }

  const OutputPort *getOutput(const boost::uuids::uuid &id) const noexcept {
    return const_cast<PortManager *>(this)->getOutput(id);
  }

private:
  map_t _ports;
  mutable std::recursive_mutex _mutex;
  boost::uuids::random_generator_pure _uuidGen;
};

} // namespace a8::midi

#endif /* A8_MIDI_H_ */
