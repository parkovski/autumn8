#ifndef A8_MIDI_H_
#define A8_MIDI_H_

#include "a8/a8.h"

#include <string_view>
#include <future>
#include <memory>
#include <mutex>
#include <exception>

namespace a8::midi {

#if defined(_WIN32)
# define A8_MIDI_BACKENDS(M) \
    M(Winmm) \
    M(UWP)
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

  using Port::Port;
};

class OutputPort : public Port {
public:
  using pointer = Port::unique_ptr<OutputPort>;
  using promise_type = std::promise<pointer>;
  using future_type = std::future<pointer>;

  using Port::Port;
};

template<typename P>
class PortManager {
public:
  using promise_type = std::promise<std::unique_ptr<PortManager>>;
  using future_type = std::future<std::unique_ptr<PortManager>>;

  friend struct iterator;
  struct iterator {
    friend class PortManager;

    iterator(const PortManager *manager, size_t index) noexcept
      : manager(manager), index(index)
    {}

    iterator(const PortManager *manager) noexcept
      : iterator(manager, -1)
    {}

    bool operator==(const iterator &other) const noexcept {
      return index == other.index;
    }
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
    iterator operator++(int) noexcept {
      iterator current = *this;
      ++*this;
      return current;
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

  static size_t iteratorIndex(const iterator &it) {
    return it.index;
  }

public:
  virtual ~PortManager();

  template<Backend B>
  static std::unique_ptr<PortManager> create();

  // \throws BackendException
  static std::unique_ptr<PortManager>
  create(Backend backend = Backend::Default);

  virtual std::future<void> scan() = 0;
  virtual void startWatching() = 0;
  virtual void stopWatching() = 0;

  virtual size_t size() const noexcept = 0;
  bool empty() const noexcept { return size() == 0; }
  // Iterator acquires a lock. Don't keep it around.
  iterator begin() const noexcept {
    if (empty()) {
      return {this};
    } else {
      return {this, 0};
    }
  }
  iterator end() const noexcept { return {this}; }
  iterator get(size_t i) const noexcept {
    if (i < size()) {
      return {this, i};
    } else {
      return {this};
    }
  }

  virtual typename P::future_type open(const iterator &it) = 0;

protected:
  std::mutex _mutex;
};

} // namespace a8::midi

#endif /* A8_MIDI_H_ */
