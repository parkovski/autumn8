#ifndef A8_MIDI_PORT_H_
#define A8_MIDI_PORT_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <future>

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
                std::string version);

public:
  virtual ~Port() = 0;

  virtual std::future<Port *> open() = 0;
  virtual std::future<Port *> close() = 0;
  void *onStateChange();

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

private:
  std::string _id;
  std::string _manufacturer;
  std::string _name;
  std::string _version;
};

class Input : public Port {
public:
  Type type() const noexcept override final;

  void onMIDIMessage();
};

class Output : public Port {
public:
  Type type() const noexcept override final;

  void send(const unsigned char *data, size_t length, uint32_t time = 0);

  //void send(std::span<unsigned char> data, uint32_t time = 0);

  void clear();
};

} // namespace a8::midi

#endif /* A8_MIDI_PORT_H_ */
