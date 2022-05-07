#ifndef A8_MIDI_PORT_H_
#define A8_MIDI_PORT_H_

#include "message.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <future>
#include <span>

namespace a8::midi {

class Port {
public:
  enum class Type {
    Input,
    Output,
  };

  enum class DeviceState {
    Disconnected,
    Connected,
  };

  enum class ConnectionState {
    Open,
    Closed,
    Pending,
  };

protected:
  explicit Port(std::string id, std::string manufacturer, std::string name,
                std::string version) noexcept
    : _id{std::move(id)},
      _manufacturer{std::move(manufacturer)},
      _name{std::move(name)},
      _version{std::move(version)}
  {
  }

public:
  virtual ~Port() = 0;

  virtual std::future<Port *> open() noexcept = 0;
  virtual std::future<Port *> close() noexcept = 0;

  void onStateChange(std::function<void (Port *)> listener) noexcept {
    _stateChangeListener = std::move(listener);
  }

  std::string_view id() const noexcept {
    return _id;
  }

  std::string_view manufacturer() const noexcept {
    return _manufacturer;
  }

  std::string_view name() const noexcept {
    return _name;
  }

  std::string_view version() const noexcept {
    return _version;
  }

  virtual Type type() const noexcept = 0;

  virtual DeviceState state() const noexcept = 0;

  virtual ConnectionState connection() const noexcept = 0;

protected:
  std::function<void (Port *)> _stateChangeListener;

private:
  std::string _id;
  std::string _manufacturer;
  std::string _name;
  std::string _version;
};

class Input : public virtual Port {
public:
  using message_type = void (Input *, uint32_t, const Message &);

  Type type() const noexcept override final;

  void onMIDIMessage(std::function<message_type> listener) {
    _listener = std::move(listener);
  }

protected:
  void emit(uint32_t time, const Message &message) {
    _listener(this, time, message);
  }

private:
  std::function<message_type> _listener;
};

class Output : public virtual Port {
public:
  Type type() const noexcept override final;

  virtual void send(const Message &message, uint32_t time = 0) = 0;

  virtual void clear() = 0;
};

} // namespace a8::midi

#endif /* A8_MIDI_PORT_H_ */
