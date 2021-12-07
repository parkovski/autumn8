#include "a8/midi/midi.h"

#include <alsa/asoundlib.h>

#include <thread>

using namespace a8;
using boost::uuids::uuid;

namespace a8::midi {

class AlsaInputPort final : public InputPort {
  snd_seq_t *_seq;
  int _port;

public:
  explicit AlsaInputPort(uuid id, snd_seq_t *seq)
    : InputPort(id, "name"),
      _seq{seq}
  {
    _port = snd_seq_create_simple_port(
      seq, "name",
      SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
      SND_SEQ_PORT_TYPE_HARDWARE
    );
  }

  ~AlsaInputPort() {
    snd_seq_delete_simple_port(_seq, _port);
  }

  bool deviceConnected() const noexcept override {
    return false;
  }

  Connection connection() const noexcept override {
    return Connection::Disconnected;
  }

  std::future<InputPort *> open() override {
    std::promise<InputPort *> promise;
    auto future = promise.get_future();
    promise.set_value(this);
    return future;
  }

  std::future<Port *> close() override {
    std::promise<Port *> promise;
    auto future = promise.get_future();
    promise.set_value(this);
    return future;
  }
};

class AlsaOutputPort final : public OutputPort {
  snd_seq_t *_seq;
  int _port;

public:
  explicit AlsaOutputPort(uuid id, snd_seq_t *seq)
    : OutputPort(id, "name"),
      _seq{seq}
  {
    _port = snd_seq_create_simple_port(
      seq, "name",
      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
      SND_SEQ_PORT_TYPE_HARDWARE
    );
  }

  ~AlsaOutputPort() {
    snd_seq_delete_simple_port(_seq, _port);
  }

  void send(const unsigned char *begin, const unsigned char *end,
            Port::timestamp_t timestamp) override {
  }

  bool deviceConnected() const noexcept override {
    return false;
  }

  Connection connection() const noexcept override {
    return Connection::Disconnected;
  }

  std::future<OutputPort *> open() override {
    std::promise<OutputPort *> promise;
    auto future = promise.get_future();
    promise.set_value(this);
    return future;
  }

  std::future<Port *> close() override {
    std::promise<Port *> promise;
    auto future = promise.get_future();
    promise.set_value(this);
    return future;
  }
};

class AlsaPortManager final : public PortManager {
  snd_seq_t *_seq = nullptr;
  int _clientId = -1;
  int _inputPortId = -1;
  std::thread _eventLoopThread;

public:
  using typename PortManager::iterator;
  using typename PortManager::const_iterator;

  explicit AlsaPortManager() {
    auto err = snd_seq_open(&_seq, "default", SND_SEQ_OPEN_DUPLEX,
                            SND_SEQ_NONBLOCK);
    if (err != 0) {
      Log::error("snd_seq_open failed with error {}", snd_strerror(err));
      _seq = nullptr;
      return;
    }
    auto closeSeq = scope_exit([this] () {
      snd_seq_close(_seq);
      _seq = nullptr;
    });
    snd_seq_set_client_name(_seq, "Autumn8" /* TODO: Put app name here */);
    _clientId = snd_seq_client_id(_seq);

    _inputPortId = snd_seq_create_simple_port(
      _seq, nullptr, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    if (_inputPortId < 0) {
      Log::error("snd_seq_create_simple_port failed: {}",
                 snd_strerror(_inputPortId));
      return;
    }

    err = subscribeToAnnouncePort();
    if (err != 0) {
      Log::error("Subscribe to announce port failed: {}", snd_strerror(err));
      return;
    }

    _eventLoopThread = std::thread(std::bind(&AlsaPortManager::eventLoop, this));
    closeSeq.release();
  }

  ~AlsaPortManager() {
    Log::debug("~AlsaPortManager()");
    if (_seq) {
      snd_seq_event_t ev;
      snd_seq_ev_clear(&ev);
      snd_seq_ev_set_source(&ev, _inputPortId);
      snd_seq_ev_set_dest(&ev, _clientId, _inputPortId);
      snd_seq_ev_set_direct(&ev);
      ev.type = SND_SEQ_EVENT_USR0;
      Log::debug("Send shutdown event");
      snd_seq_event_output_direct(_seq, &ev);
      Log::debug("_eventLoopThread.join()");
      _eventLoopThread.join();
      Log::debug("snd_seq_close");
      snd_seq_close(_seq);
    }
  }

private:
  int subscribeToAnnouncePort() {
    snd_seq_port_subscribe_t *sub;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_addr_t announceAddr;
    announceAddr.client = SND_SEQ_CLIENT_SYSTEM;
    announceAddr.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
    snd_seq_addr_t inputPortAddr;
    inputPortAddr.client = _clientId;
    inputPortAddr.port = _inputPortId;
    snd_seq_port_subscribe_set_sender(sub, &announceAddr);
    snd_seq_port_subscribe_set_dest(sub, &inputPortAddr);
    return snd_seq_subscribe_port(_seq, sub);
  }

  void eventLoop() {
    int pollDescriptors = snd_seq_poll_descriptors_count(_seq, POLLIN);
    struct pollfd *pollfds =
      (struct pollfd *)alloca(sizeof(struct pollfd) * pollDescriptors);
    snd_seq_poll_descriptors(_seq, pollfds, (int)pollDescriptors, POLLIN);

    while (true) {
      // TODO Chrome deals with EINTR here.
      int err = poll(pollfds, (nfds_t)pollDescriptors, -1);
      if (err == -1) {
        if (errno == EINTR) {
          Log::debug("MIDI event loop EINTR");
          return;
        } else {
          Log::error("MIDI event loop poll failed: {}", strerror(errno));
          return;
        }
      }

      bool pending;
      do {
        snd_seq_event_t *event;
        err = snd_seq_event_input(_seq, &event);
        pending = snd_seq_event_input_pending(_seq, 0) > 0;
        if (err == -ENOSPC) {
          Log::warn("MIDI event loop ENOSPC");
        } else if (err == -EAGAIN) {
          Log::warn("MIDI event loop EAGAIN");
          break;
        } else if (err < 0) {
          Log::error("MIDI event loop input: {}", strerror(errno));
          return;
        } else if (event->source.client == _clientId &&
                   event->type == SND_SEQ_EVENT_USR0) {
          Log::debug("MIDI event loop shutdown");
          return;
        } else if (event->source.client == SND_SEQ_CLIENT_SYSTEM &&
                   event->source.port == SND_SEQ_PORT_SYSTEM_ANNOUNCE) {
          switch (event->type) {
            case SND_SEQ_EVENT_PORT_START:
              Log::info("MIDI port start {}:{}", event->data.addr.client,
                        event->source.port);
              break;

            case SND_SEQ_EVENT_PORT_EXIT:
              Log::info("MIDI port exit {}:{}", event->data.addr.client,
                        event->data.addr.port);
              break;

            case SND_SEQ_EVENT_CLIENT_START:
              {
                snd_seq_client_info_t *ci;
                snd_seq_client_info_alloca(&ci);
                snd_seq_get_any_client_info(_seq, event->data.addr.client, ci);
                auto name = snd_seq_client_info_get_name(ci);
                Log::info("MIDI client start {} {}", event->data.addr.client, name);
              }
              break;

            case SND_SEQ_EVENT_CLIENT_EXIT:
              Log::info("MIDI client exit {}", event->data.addr.client);
              break;
          }
        } else {
          Log::debug("MIDI event {}:{} Ch{}", event->source.client,
                     event->source.port, (int)event->data.control.channel);
        }
      } while (pending);
    }
  }
};

template<>
std::unique_ptr<PortManager>
PortManager::createInternal<Backend::Alsa>() {
  return std::make_unique<AlsaPortManager>();
}

} // namespace a8::midi
