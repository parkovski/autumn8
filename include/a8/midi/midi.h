#ifndef A8_MIDI_H_
#define A8_MIDI_H_

#include "a8/a8.h"

#include <string_view>
#include <future>
#include <memory>
#include <mutex>
#include <exception>

namespace a8::midi {

template<typename T>
using unique_promise = std::promise<std::unique_ptr<T>>;

template<typename T>
using unique_future = std::future<std::unique_ptr<T>>;

enum class Backend {
  Winmm,
  UWP,

#ifdef _WIN32
  Default = Winmm
#endif
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

class Port {
protected:
  explicit Port(std::string name)
    : _name{std::move(name)}
  {}

public:
  virtual ~Port() = 0;

  std::string_view name() const { return _name; }

private:
  std::string _name;
};

class InputPort : public Port {
public:
  using promise_type = unique_promise<InputPort>;
  using future_type = unique_future<InputPort>;

  using Port::Port;
};

class OutputPort : public Port {
public:
  using promise_type = unique_promise<OutputPort>;
  using future_type = unique_future<OutputPort>;

  using Port::Port;
};

template<typename P>
class PortManager {
public:
  using promise_type = unique_promise<PortManager>;
  using future_type = unique_future<PortManager>;

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

void uwpMidiInitWinRT();

} // namespace a8::midi

#endif /* A8_MIDI_H_ */
