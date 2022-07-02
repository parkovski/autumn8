#ifndef A8_MIDI_ALSA_H_
#define A8_MIDI_ALSA_H_

#include "a8/midi/access.h"
#include "a8/midi/port.h"
#include "a8/midi/message.h"

#include <alsa/asoundlib.h>

#include <iosfwd>

namespace a8::midi {
inline namespace alsaseq {

Message to_message(const snd_seq_event_t &ev) noexcept;
snd_seq_event_t from_message(const Message &msg) noexcept;

class AlsaAccess : public Access {
  static constexpr size_t CmdLength =
    sizeof(std::declval<snd_seq_event_t>().data.raw8.d);
  static constexpr const char ShutdownCmd[CmdLength + 1] = "AlsaShutdown";

  static int subscribeToAnnouncePort(snd_seq_t *seq, unsigned char clientId,
                                     unsigned char inputPortId) noexcept;
  void collectPorts() noexcept;
  void isInOutPort(snd_seq_client_info_t *ci, snd_seq_port_info_t *pi,
                   bool &input, bool &output) noexcept;
  bool internalEvent(snd_seq_event_t *ev) noexcept;
  void inputPortEvent(snd_seq_event_t *ev) noexcept;
  void announcePortEvent(snd_seq_event_t *ev) noexcept;
  void generalEvent(snd_seq_event_t *ev) noexcept;
  void eventLoop() noexcept;

public:
  explicit AlsaAccess(Options options, snd_seq_t *seq, unsigned char clientId,
                      unsigned char inputPortId);

  ~AlsaAccess();

  static void create(std::promise<std::shared_ptr<Access>> promise,
                     Options options);

private:
  snd_seq_t *_seq;
  int _clientId;
  int _inputPortId;
  std::thread _eventLoopThread;
};

class AlsaPort : public virtual Port {
protected:
  static std::string port_id(snd_seq_port_info_t *pi);
  static std::string client_port_name(snd_seq_client_info_t *ci,
                                      snd_seq_port_info_t *pi);

  explicit AlsaPort(snd_seq_t *seq, snd_seq_addr_t addr, unsigned char port)
                    noexcept;

public:
  DeviceState state() const noexcept override final;
  ConnectionState connection() const noexcept override final;

protected:
  snd_seq_t *_seq;
  snd_seq_addr_t _addr;
  unsigned char _port;
};

class AlsaInput final : public AlsaPort, public Input {
  friend class AlsaAccess;

  void unsubscribe() noexcept;

public:
  explicit AlsaInput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                     snd_seq_port_info_t *pi);

  ~AlsaInput();

  std::future<Port *> open() noexcept override;
  std::future<Port *> close() noexcept override;

private:
  snd_seq_port_subscribe_t *_sub = nullptr;
};

class AlsaOutput final : public AlsaPort, public Output {
public:
  explicit AlsaOutput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                      snd_seq_port_info_t *pi);

  ~AlsaOutput();

  std::future<Port *> open() noexcept override;
  std::future<Port *> close() noexcept override;

  void send(const Message &message, uint32_t time = 0) override;

  void clear() override;
};

struct AlsaPortCaps {
  unsigned value;
};
std::ostream &operator<<(std::ostream &os, AlsaPortCaps caps);

struct AlsaPortType {
  unsigned value;
};
std::ostream &operator<<(std::ostream &os, AlsaPortType type);

struct AlsaEventType {
  unsigned value;
};
std::ostream &operator<<(std::ostream &os, AlsaEventType type);

} // inline namespace alsaseq
} // namespace a8::midi

#endif /* A8_MIDI_ALSA_H_ */
