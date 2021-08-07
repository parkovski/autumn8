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
#include <optional>

namespace a8::midi {

#if defined(_WIN32)
# define A8_MIDI_BACKENDS(M) \
    M(Winmm) \
    //M(UWP)
# define A8_MIDI_DEFAULT_BACKEND Winmm
#elif defined(__ANDROID__)
# define A8_MIDI_BACKENDS(M) M(Android)
# define A8_MIDI_DEFAULT_BACKEND Android
#endif

// MIDI backends available on this platform.
enum class Backend {
#define A8_MIDI_BE(be) be,
  A8_MIDI_BACKENDS(A8_MIDI_BE)
#undef A8_MIDI_BE
  Default = A8_MIDI_DEFAULT_BACKEND
};

std::ostream &operator<<(std::ostream &os, Backend be);

class BackendException : public std::runtime_error {
public:
  BackendException(Backend backend) noexcept
    : std::runtime_error("MIDI backend does not exist"),
      _backend{backend}
  {}

  Backend backend() const noexcept {
    return _backend;
  }

private:
  Backend _backend;
};


// Common MIDI port base type.
class Port {
public:
  friend struct async_destroy;
  struct async_destroy {
    void operator()(Port *port) const {
      port->destroy();
    }
  };

  // Specialization of \c std::unique_ptr for async destruction.
  template<typename T = Port>
  using unique_ptr = std::unique_ptr<T, async_destroy>;

  // Substitute for \c std::make_unique using the \c async_destroy deleter.
  template<typename T, typename... Args>
  static unique_ptr<T> make_unique(Args &&...args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

protected:
  explicit Port(std::string name)
    : _name{std::move(name)}
  {}

  virtual ~Port();
  virtual void destroy() = 0;

public:
  static std::future<void> destroy(unique_ptr<> self) {
    return self->_destroyPromise.get_future();
  }
  std::string_view name() const { return _name; }

private:
  std::promise<void> _destroyPromise;
  std::string _name;
};

class InputPort : public Port {
public:
  using pointer = Port::unique_ptr<InputPort>;
  using promise_type = std::promise<pointer>;
  using future_type = std::future<pointer>;
  using sink_type = std::function<void(const IMessage &, uint32_t)>;

  using Port::Port;

  void sink(sink_type sink) {
    if (sink) {
      _sink = std::move(sink);
    } else {
      _sink = &InputPort::nop;
    }
  }

  void start(sink_type sink) {
    this->sink(std::move(sink));
    start();
  }

  virtual void start() = 0;
  virtual void stop() = 0;

protected:
  void emit(const IMessage &message, uint32_t timestamp) const {
    _sink(message, timestamp);
  }

private:
  static void nop(const IMessage &, uint32_t) {}

  sink_type _sink = &InputPort::nop;
};

class OutputPort : public Port {
public:
  using pointer = Port::unique_ptr<OutputPort>;
  using promise_type = std::promise<pointer>;
  using future_type = std::future<pointer>;

  using Port::Port;

  //virtual void operator()(unsigned char *data, size_t length) = 0;
};

template<typename P>
class PortManager {
public:
  using promise_type = std::promise<std::unique_ptr<PortManager>>;
  using future_type = std::future<std::unique_ptr<PortManager>>;

  friend struct iterator;
  struct iterator {
    friend class PortManager;

  private:
    // End iterator - does not hold a lock.
    iterator() noexcept
      : iterator(nullptr, -1)
    {}

    // Important! Must have a lock before calling this.
    iterator(const PortManager *manager, size_t index) noexcept
      : manager(manager), index(index)
    {}

  public:
    // Releases the lock held on the \c PortManager if needed.
    ~iterator() {
      if (manager) manager->_mutex.unlock();
    }

    // Comparisons are only valid for iterators from the same container.
    bool operator==(const iterator &other) const noexcept {
      return index == other.index;
    }

    // Comparisons are only valid for iterators from the same container.
    bool operator!=(const iterator &other) const noexcept {
      return !(*this == other);
    }

    iterator &operator++() noexcept {
      if (index < manager->size() - 1) {
        ++index;
      } else {
        index = -1;
      }
      return *this;
    }
    /*const Info &operator*() const noexcept {
      return manager->info(id);
    }
    const Info *operator->() const noexcept {
      return &operator*();
    }*/

  private:
    const PortManager *manager;
    size_t index;
  };

protected:
  PortManager() = default;

  /// Acquires a lock and returns an iterator for port at \c index.
  iterator lockedIterator(size_t index) const {
    _mutex.lock();
    return iterator{this, index};
  }

  /// Tries to acquire a lock and returns an iterator for port at \c index,
  /// or \c std::nullopt if the lock was not successful.
  std::optional<iterator> tryLockedIterator(size_t index) const {
    if (_mutex.try_lock()) {
      return iterator{this, index};
    }
    return std::nullopt;
  }

  /// \returns The index pointed to by \c it. The index is -1 if \c it
  /// is an end iterator.
  static size_t iteratorIndex(const iterator &it) {
    return it.index;
  }

public:
  virtual ~PortManager();

  template<Backend B>
  static std::unique_ptr<PortManager> create();

  /// \throws BackendException
  static std::unique_ptr<PortManager>
  create(Backend backend = Backend::Default);

  virtual std::future<void> scan() = 0;
  virtual void startWatching() = 0;
  virtual void stopWatching() = 0;

  // Returns the current number of accessible ports. The size may change if a
  // locked iterator is not held.
  virtual size_t size() const noexcept = 0;

  /// Returns true if the port collection is currently empty. \see size().
  bool empty() const noexcept { return size() == 0; }

  /// If there are no ports, returns an end iterator. Otherwise, locks the
  /// \c PortManager and returns an iterator to the first element.
  iterator begin() const {
    if (empty()) {
      return end();
    } else {
      return lockedIterator(0);
    }
  }

  /// If there are no ports, returns an end iterator. Otherwise, tries to lock
  /// the \c PortManager. If the lock was successful, returns an iterator to
  /// the first element. If not, returns \c std::nullopt.
  std::optional<iterator> tryBegin() const {
    if (empty()) {
      return end();
    } else {
      return tryLockedIterator(0);
    }
  }

  /// \returns End iterator. Does not hold a lock.
  iterator end() const noexcept { return {}; }

  /// Returns an iterator to element \c i. The iterator holds a lock on the
  /// \c PortManager.
  iterator get(size_t i) const {
    if (i < size()) {
      return lockedIterator(i);
    } else {
      return end();
    }
  }

  /// Returns an iterator to element \c i, or \c std::nullopt if a lock could
  /// not be acquired.
  std::optional<iterator> tryGet(size_t i) const {
    if (i < size()) {
      return tryLockedIterator(i);
    } else {
      return end();
    }
  }

  /// Opens the port pointed to by \c it.
  virtual typename P::future_type open(const iterator &it) = 0;

protected:
  mutable std::mutex _mutex;
};

} // namespace a8::midi

#endif /* A8_MIDI_H_ */
