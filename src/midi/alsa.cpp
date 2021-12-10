#include "a8/midi/midi.h"

#include <alsa/asoundlib.h>

#include <thread>

using namespace a8;

namespace a8::midi {

constexpr unsigned PMCMDLENGTH = 12;
static_assert(sizeof(std::declval<snd_seq_event_t>().data.raw8.d) ==
              PMCMDLENGTH);
constexpr const char *const PMCMD_Shutdown = "A8PMShutdown";

struct AlsaPortCaps {
  unsigned caps;
  AlsaPortCaps(unsigned caps) noexcept : caps{caps} {}
  operator unsigned() const noexcept { return caps; }
};

struct AlsaPortType {
  unsigned type;
  AlsaPortType(unsigned type) noexcept : type{type} {}
  operator unsigned() const noexcept { return type; }
};

#define PRINTFLAG(Kind, Flag)               \
  do {                                      \
    if (v & SND_SEQ_PORT_##Kind##_##Flag) { \
      if (first) {                          \
        first = false;                      \
      } else {                              \
        os << ", ";                         \
      }                                     \
      os << #Flag;                          \
    }                                       \
  } while (0)

std::ostream &operator<<(std::ostream &os, AlsaPortCaps caps) {
  unsigned v = caps;
  if (v == 0) {
    return os << "<NONE>";
  }
  bool first = true;
  PRINTFLAG(CAP, READ);
  PRINTFLAG(CAP, WRITE);
  PRINTFLAG(CAP, SYNC_READ);
  PRINTFLAG(CAP, SYNC_WRITE);
  PRINTFLAG(CAP, DUPLEX);
  PRINTFLAG(CAP, SUBS_READ);
  PRINTFLAG(CAP, SUBS_WRITE);
  PRINTFLAG(CAP, NO_EXPORT);
  return os;
}

std::ostream &operator<<(std::ostream &os, AlsaPortType type) {
  unsigned v = type;
  if (v == 0) {
    return os << "<NONE>";
  }
  bool first = true;
  PRINTFLAG(TYPE, SPECIFIC);
  PRINTFLAG(TYPE, MIDI_GENERIC);
  PRINTFLAG(TYPE, MIDI_GM);
  PRINTFLAG(TYPE, MIDI_GS);
  PRINTFLAG(TYPE, MIDI_XG);
  PRINTFLAG(TYPE, MIDI_MT32);
  PRINTFLAG(TYPE, MIDI_GM2);
  PRINTFLAG(TYPE, SYNTH);
  PRINTFLAG(TYPE, DIRECT_SAMPLE);
  PRINTFLAG(TYPE, SAMPLE);
  PRINTFLAG(TYPE, HARDWARE);
  PRINTFLAG(TYPE, SOFTWARE);
  PRINTFLAG(TYPE, SYNTHESIZER);
  PRINTFLAG(TYPE, PORT);
  PRINTFLAG(TYPE, APPLICATION);
  return os;
}

#undef PRINTFLAG

Port::id_t alsa_addr_to_port_id(snd_seq_addr_t addr) noexcept {
  return (((Port::id_t)addr.port) << 16) | addr.client;
}

snd_seq_addr_t port_id_to_alsa_addr(Port::id_t id) noexcept {
  snd_seq_addr_t addr;
  addr.port = (unsigned char)(id >> 16);
  addr.client = (unsigned char)(id & 0xFFFF);
  return addr;
}

class AlsaInputPort final : public InputPort {
  snd_seq_t *_seq;
  snd_seq_addr_t _subscribeAddr;
  snd_seq_port_subscribe_t *_sub = nullptr;

public:
  explicit AlsaInputPort(snd_seq_t *seq, snd_seq_port_info_t *pi) noexcept
    : InputPort(alsa_addr_to_port_id(*snd_seq_port_info_get_addr(pi)),
                snd_seq_port_info_get_name(pi)),
      _seq{seq}
  {}

  ~AlsaInputPort() {}

  bool deviceConnected() const noexcept override {
    return false;
  }

  Connection connection() const noexcept override {
    return Connection::Disconnected;
  }

  std::future<InputPort *> open() override {
    std::promise<InputPort *> promise;
    auto future = promise.get_future();
    snd_seq_port_subscribe_t *sub;
    snd_seq_port_subscribe_alloca(&sub);
    auto sourcePortId = port_id_to_alsa_addr(id());
    snd_seq_port_subscribe_set_sender(sub, &sourcePortId);
    snd_seq_port_subscribe_set_dest(sub, &_subscribeAddr);
    snd_seq_subscribe_port(_seq, sub);
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
  unsigned char _sourcePort;

public:
  explicit AlsaOutputPort(snd_seq_t *seq, snd_seq_port_info_t *pi) noexcept
    : OutputPort(alsa_addr_to_port_id(*snd_seq_port_info_get_addr(pi)),
                 snd_seq_port_info_get_name(pi)),
      _seq{seq}
  {}

  ~AlsaOutputPort() {}

  void send(const unsigned char *begin, const unsigned char *end,
            Port::timestamp_t timestamp) override {
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    auto addr = port_id_to_alsa_addr(id());
    snd_seq_ev_set_dest(&ev, addr.client, addr.port);
    snd_seq_event_output(_seq, &ev);
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
      _seq, "Input", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    if (_inputPortId < 0) {
      Log::error("snd_seq_create_simple_port failed: {}",
                 snd_strerror(_inputPortId));
      return;
    }

    err = subscribeToAnnouncePort();
    if (err != 0) {
      Log::error("Subscribe to announce port failed: {}", snd_strerror(-err));
      return;
    }
    collectPorts();

    _eventLoopThread = std::thread(std::bind(&AlsaPortManager::eventLoop,
                                             this));
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
      memcpy(ev.data.raw8.d, "A8PMShutdown", sizeof(ev.data.raw8.d));
      Log::debug("Send shutdown event");
      snd_seq_event_output_direct(_seq, &ev);
      Log::debug("_eventLoopThread.join()");
      _eventLoopThread.join();
      Log::debug("snd_seq_close");
      snd_seq_close(_seq);
    }
  }

private:
  int subscribeToAnnouncePort() noexcept {
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

  void collectPorts() noexcept {
    snd_seq_client_info_t *ci;
    snd_seq_client_info_alloca(&ci);
    snd_seq_port_info_t *pi;
    snd_seq_port_info_alloca(&pi);
    auto portMapLock = lock();

    snd_seq_client_info_set_client(ci, -1);
    while (!snd_seq_query_next_client(_seq, ci)) {
      int clientId = snd_seq_client_info_get_client(ci);
      snd_seq_port_info_set_client(pi, clientId);
      snd_seq_port_info_set_port(pi, -1);
      while (!snd_seq_query_next_port(_seq, pi)) {
        processNewPort(ci, pi);
      }
    }
  }

  // Requires a lock to be held!
  void processNewPort(snd_seq_client_info_t *ci, snd_seq_port_info_t *pi)
      noexcept {
    static constexpr int portCapInput =
      SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
    static constexpr int portCapOutput =
      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
    int portId = snd_seq_port_info_get_port(pi);
    unsigned cap = snd_seq_port_info_get_capability(pi);
    const char *name = snd_seq_port_info_get_name(pi);
    unsigned type = snd_seq_port_info_get_type(pi);
    const char *clientName = snd_seq_client_info_get_name(ci);
    int clientId = snd_seq_client_info_get_client(ci);

    if (clientId == _clientId) {
      Log::trace("Skip own MIDI port {}: {} ({}:{}) [{}; {}]", clientName,
                 name, clientId, portId, AlsaPortCaps{cap},
                 AlsaPortType{type});
      return;
    }
    if (!(type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)) {
      Log::trace("Skip non-MIDI_GENERIC port {}: {} ({}:{}) [{}; {}]",
                 clientName, name, clientId, portId, AlsaPortCaps{cap},
                 AlsaPortType{type});
      return;
    }
    if (!(cap & (portCapInput | portCapOutput))) {
      Log::trace("Skip non-read/write MIDI port {}: {} ({}:{}) [{}; {}]",
                 clientName, name, clientId, portId, AlsaPortCaps{cap},
                 AlsaPortType{type});
      return;
    }

    Log::debug("Add MIDI port {}: {} ({}:{}) [{}; {}]", clientName, name,
               clientId, portId, AlsaPortCaps{cap}, AlsaPortType{type});

    if (cap & portCapInput) {
      std::shared_ptr<Port> port = std::make_shared<AlsaInputPort>(_seq, pi);
      auto id = port->id();
      portMap().emplace(std::make_pair(id, std::move(port)));
    }
    if (cap & portCapOutput) {
      std::shared_ptr<Port> port = std::make_shared<AlsaOutputPort>(_seq, pi);
      auto id = port->id();
      portMap().emplace(std::make_pair(id, std::move(port)));
    }
  }

  // Requires a lock to be held!
  void processNewPort(snd_seq_addr_t addr) {
    snd_seq_client_info_t *ci;
    snd_seq_client_info_alloca(&ci);
    snd_seq_get_any_client_info(_seq, addr.client, ci);
    snd_seq_port_info_t *pi;
    snd_seq_port_info_alloca(&pi);
    snd_seq_get_any_port_info(_seq, addr.client, addr.port, pi);
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
          Log::warn("MIDI event loop EINTR");
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
          Log::error("MIDI event loop input: {}", strerror(-err));
          return;
        } else if (event->source.client == _clientId &&
                   event->source.port == _inputPortId &&
                   event->type == SND_SEQ_EVENT_USR0) {
          if (!memcmp(event->data.raw8.d, PMCMD_Shutdown, PMCMDLENGTH)) {
            Log::debug("MIDI event loop shutdown");
            return;
          } else {
            char data[PMCMDLENGTH + 1];
            memcpy(data, event->data.raw8.d, PMCMDLENGTH);
            data[PMCMDLENGTH] = 0;
            Log::warn("Unknown internal MIDI event '{}'.", data);
          }
        } else if (event->source.client == SND_SEQ_CLIENT_SYSTEM &&
                   event->source.port == SND_SEQ_PORT_SYSTEM_ANNOUNCE) {
          snd_seq_client_info_t *ci;
          snd_seq_client_info_alloca(&ci);
          snd_seq_get_any_client_info(_seq, event->data.addr.client, ci);
          auto name = snd_seq_client_info_get_name(ci);
          switch (event->type) {
            case SND_SEQ_EVENT_PORT_START: {
              Log::debug("MIDI port start {} ({}):{}", name,
                        event->data.addr.client, event->data.addr.port);
              auto portMapLock = lock();
              processNewPort(event->data.addr);
              break;
            }

            case SND_SEQ_EVENT_PORT_EXIT:
              Log::debug("MIDI port exit {} ({}):{}", name,
                         event->data.addr.client, event->data.addr.port);
              break;

            case SND_SEQ_EVENT_CLIENT_START:
              Log::debug("MIDI client start {} ({})", name,
                         event->data.addr.client);
              break;

            case SND_SEQ_EVENT_CLIENT_EXIT:
              Log::debug("MIDI client exit {} ({})", name,
                         event->data.addr.client);
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
