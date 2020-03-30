#ifndef A8_MIDI_H_
#define A8_MIDI_H_

#include "a8/a8.h"

#include <functional>
#include <string_view>
#include <future>
#include <atomic>
#include <variant>
#include <optional>
#include <memory>
#include <mutex>

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

class Port {
protected:
  explicit Port();

public:
  std::string_view name() const;
};

class InputPort : public Port {
public:
  using promise_type = unique_promise<InputPort>;
  using future_type = unique_future<InputPort>;

  virtual ~InputPort() = 0;
};

class OutputPort : public Port {
public:
  using promise_type = unique_promise<OutputPort>;
  using future_type = unique_future<OutputPort>;

  virtual ~OutputPort() = 0;
};

template<typename P>
class PortManager {
public:
  using promise_type = unique_promise<PortManager>;
  using future_type = unique_future<PortManager>;

  friend struct iterator;
  struct iterator {
    iterator(const PortManager *manager) noexcept
      : manager(manager), index(-1)
    {}

    iterator(const PortManager *manager, size_t index) noexcept
      : manager(manager), index(index)
    {}

    bool operator==(const iterator &other) const noexcept {
      return index == other.index;
    }
    bool operator!=(const iterator &other) const noexcept {
      return !(*this == other);
    }

    iterator &operator++() noexcept {
      if (manager->hasNext(index)) {
        ++index;
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

  virtual bool hasNext(size_t index) const noexcept = 0;

public:
  virtual ~PortManager();

  template<Backend B>
  static future_type create();

  static future_type create(Backend backend = Backend::Default);

  // Iterator acquires a lock. Don't keep it around.
  virtual iterator begin() const = 0;
  iterator end() const noexcept { return {this}; }

  virtual typename P::future_type open() = 0;

protected:
  std::mutex _mutex;
};

void uwpMidiInitWinRT();

} // namespace a8::midi

#endif /* A8_MIDI_H_ */
