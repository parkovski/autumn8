#include "a8/a8.h"
#include "a8/midi/access.h"
#include "a8/midi/port.h"

#include <alsa/asoundlib.h>

#include <thread>
#include <exception>
#include <sstream>

namespace a8::midi {

struct AlsaPortCaps {
  unsigned caps;
  AlsaPortCaps(unsigned caps) noexcept : caps{caps} {}
  //operator unsigned() const noexcept { return caps; }
};

struct AlsaPortType {
  unsigned type;
  AlsaPortType(unsigned type) noexcept : type{type} {}
  //operator unsigned() const noexcept { return type; }
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
  unsigned v = caps.caps;
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
  unsigned v = type.type;
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

  void send(std::span<unsigned char> data, uint32_t time = 0) override;

  void clear() override;
};

class AlsaAccess : public Access {
  static constexpr size_t CmdLength = 12;
  static_assert(
    sizeof(std::declval<snd_seq_event_t>().data.raw8.d) == CmdLength);
  static constexpr const char *const ShutdownCmd = "AlsaShutdown";

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

// AlsaPort {{{
std::string AlsaPort::port_id(snd_seq_port_info_t *pi) {
  const snd_seq_addr_t *addr = snd_seq_port_info_get_addr(pi);
  std::stringstream s;
  s << (int)addr->client << ":" << (int)addr->port;
  return s.str();
}

std::string AlsaPort::client_port_name(snd_seq_client_info_t *ci,
                                       snd_seq_port_info_t *pi) {
  std::stringstream name;
  name << snd_seq_client_info_get_name(ci)
       << ": "
       << snd_seq_port_info_get_name(pi);
  return name.str();
}

AlsaPort::AlsaPort(snd_seq_t *seq, snd_seq_addr_t addr, unsigned char port)
                   noexcept
  : _seq{seq},
    _addr{addr},
    _port{port}
{}

auto AlsaPort::state() const noexcept -> DeviceState {
  return DeviceState::Disconnected;
}

auto AlsaPort::connection() const noexcept -> ConnectionState {
  return ConnectionState::Closed;
}
// AlsaPort }}}

// AlsaInput {{{
AlsaInput::AlsaInput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                     snd_seq_port_info_t *pi)
  : AlsaPort(seq,
             *snd_seq_port_info_get_addr(pi),
             (unsigned char)snd_seq_port_info_get_port(pi)),
    Port(port_id(pi),
         "Unknown", // Manufacturer
         client_port_name(ci, pi),
         "Unknown") // Version
{}

void AlsaInput::unsubscribe() noexcept {
  snd_seq_unsubscribe_port(_seq, _sub);
  snd_seq_port_subscribe_free(_sub);
  _sub = nullptr;
}

AlsaInput::~AlsaInput() {
  if (_sub) {
    unsubscribe();
  }
}

std::future<Port *> AlsaInput::open() noexcept {
  std::promise<Port *> promise;

  if (_sub) {
    promise.set_value(this);
    return promise.get_future();
  }

  snd_seq_port_subscribe_malloc(&_sub);
  snd_seq_port_subscribe_set_sender(_sub, &_addr);

  snd_seq_client_info_t *clientInfo;
  snd_seq_client_info_alloca(&clientInfo);
  snd_seq_get_client_info(_seq, clientInfo);

  snd_seq_addr_t subscribeAddr;
  subscribeAddr.client =
    (unsigned char)snd_seq_client_info_get_client(clientInfo);
  subscribeAddr.port = _port;
  snd_seq_port_subscribe_set_dest(_sub, &subscribeAddr);

  if (int err = snd_seq_subscribe_port(_seq, _sub)) {
    promise.set_exception(
      std::make_exception_ptr(
        std::system_error(-err, std::system_category())));
  }

  // TODO subs_start
  promise.set_value(this);
  return promise.get_future();
}

std::future<Port *> AlsaInput::close() noexcept {
  if (_sub) {
    unsubscribe();
  }
  std::promise<Port *> promise;
  promise.set_value(this);
  return promise.get_future();
}
// AlsaInput }}}

// AlsaOutput {{{
AlsaOutput::AlsaOutput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                       snd_seq_port_info_t *pi)
  : AlsaPort(seq,
             *snd_seq_port_info_get_addr(pi),
             (unsigned char)snd_seq_port_info_get_port(pi)),
    Port(port_id(pi),
         "Unknown", // Manufacturer
         client_port_name(ci, pi),
         "Unknown") // Version
{}

AlsaOutput::~AlsaOutput() {
}

std::future<Port *> AlsaOutput::open() noexcept {
  std::promise<Port *> promise;
  promise.set_value(this);
  return promise.get_future();
}

std::future<Port *> AlsaOutput::close() noexcept {
  std::promise<Port *> promise;
  promise.set_value(this);
  return promise.get_future();
}

void AlsaOutput::send(std::span<unsigned char> data, uint32_t time) {
#if 0
  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_dest(&ev, _addr.client, _addr.port);

  if (length <= sizeof(ev.data.raw8.d)) {
    ev.flags = SND_SEQ_EVENT_LENGTH_FIXED;
    memcpy(ev.data.raw8.d, data, length);
  } else {
    if (time == 0) {
      // Uses user space pointer.
      ev.flags = SND_SEQ_EVENT_LENGTH_VARUSR;
    } else {
      // Copies data into kernel space.
      ev.flags = SND_SEQ_EVENT_LENGTH_VARIABLE;
    }
    ev.data.ext.len = static_cast<unsigned>(length);
    ev.data.ext.ptr = const_cast<unsigned char *>(data);
  }

  snd_seq_event_output(_seq, &ev);
#endif
  Log::error("MIDI send not implemented");
}

void AlsaOutput::clear() {
  Log::warn("MIDI queue not implemented");
}
// AlsaOutput }}}

// AlsaAccess {{{
int AlsaAccess::subscribeToAnnouncePort(snd_seq_t *seq, unsigned char clientId,
                                        unsigned char inputPortId) noexcept {
  snd_seq_port_subscribe_t *sub;
  snd_seq_port_subscribe_alloca(&sub);

  snd_seq_addr_t announceAddr;
  announceAddr.client = SND_SEQ_CLIENT_SYSTEM;
  announceAddr.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
  snd_seq_port_subscribe_set_sender(sub, &announceAddr);

  snd_seq_addr_t inputPortAddr;
  inputPortAddr.client = clientId;
  inputPortAddr.port = inputPortId;
  snd_seq_port_subscribe_set_dest(sub, &inputPortAddr);

  return snd_seq_subscribe_port(seq, sub);
}

void AlsaAccess::collectPorts() noexcept {
  snd_seq_client_info_t *ci;
  snd_seq_client_info_alloca(&ci);
  snd_seq_port_info_t *pi;
  snd_seq_port_info_alloca(&pi);

  auto [inputLock, inputs] = inputsWrite();
  auto [outputLock, outputs] = outputsWrite();

  snd_seq_client_info_set_client(ci, -1);
  while (!snd_seq_query_next_client(_seq, ci)) {
    int clientId = snd_seq_client_info_get_client(ci);
    snd_seq_port_info_set_client(pi, clientId);
    snd_seq_port_info_set_port(pi, -1);
    while (!snd_seq_query_next_port(_seq, pi)) {
      bool isIn, isOut;
      isInOutPort(ci, pi, isIn, isOut);
      if (isIn) {
        auto port = std::make_shared<AlsaInput>(_seq, ci, pi);
        inputs.emplace(port->id(), std::move(port));
      }
      if (isOut) {
        auto port = std::make_shared<AlsaOutput>(_seq, ci, pi);
        outputs.emplace(port->id(), std::move(port));
      }
    }
  }
}

void AlsaAccess::isInOutPort(snd_seq_client_info_t *ci,
                             snd_seq_port_info_t *pi,
                             bool &input, bool &output) noexcept {
  constexpr int portCapInput =
    SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
  constexpr int portCapOutput =
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
    input = output = false;
    return;
  }
  if (!(type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)) {
    Log::trace("Skip non-MIDI_GENERIC port {}: {} ({}:{}) [{}; {}]",
               clientName, name, clientId, portId, AlsaPortCaps{cap},
               AlsaPortType{type});
    input = output = false;
    return;
  }
  if (!(cap & (portCapInput | portCapOutput))) {
    Log::trace("Skip non-read/write MIDI port {}: {} ({}:{}) [{}; {}]",
               clientName, name, clientId, portId, AlsaPortCaps{cap},
               AlsaPortType{type});
    input = output = false;
    return;
  }

  input = (cap & portCapInput) != 0;
  output = (cap & portCapOutput) != 0;

  if (input | output) {
    Log::trace("MIDI {} port {}: {} ({}:{}) [{}; {}]",
               input ? output ? "in/out" : "in" : "out", clientName, name,
               clientId, portId, AlsaPortCaps{cap}, AlsaPortType{type});
  }
}

bool AlsaAccess::internalEvent(snd_seq_event_t *ev) noexcept {
  if (!memcmp(ev->data.raw8.d, ShutdownCmd, CmdLength)) {
    Log::trace("MIDI event loop shutdown");
    return false;
  } else {
    char data[CmdLength + 1];
    memcpy(data, ev->data.raw8.d, CmdLength);
    data[CmdLength] = 0;
    Log::warn("Unknown internal MIDI event '{}'.", data);
  }
  return true;
}

void AlsaAccess::inputPortEvent(snd_seq_event_t *ev) noexcept {
  Log::debug("Input port event {}", ev->type);
}

void AlsaAccess::announcePortEvent(snd_seq_event_t *ev) noexcept {
  snd_seq_client_info_t *ci;
  snd_seq_client_info_alloca(&ci);
  snd_seq_get_any_client_info(_seq, ev->data.addr.client, ci);
  auto name = snd_seq_client_info_get_name(ci);

  switch (ev->type) {
    case SND_SEQ_EVENT_PORT_START: {
      Log::debug("MIDI port start {} ({}):{}", name, ev->data.addr.client,
                 ev->data.addr.port);

      snd_seq_port_info_t *pi;
      snd_seq_port_info_alloca(&pi);
      snd_seq_get_any_port_info(_seq, ev->data.addr.client, ev->data.addr.port,
                                pi);
      bool isIn, isOut;
      isInOutPort(ci, pi, isIn, isOut);
      if (isIn) {
        auto [inputLock, inputs] = inputsWrite();
        auto port = std::make_shared<AlsaInput>(_seq, ci, pi);
        inputs.emplace(port->id(), std::move(port));
      }
      if (isOut) {
        auto [outputLock, outputs] = outputsWrite();
        auto port = std::make_shared<AlsaOutput>(_seq, ci, pi);
        outputs.emplace(port->id(), std::move(port));
      }
      break;
    }

    case SND_SEQ_EVENT_PORT_EXIT:
      // TODO Remove port from maps.
      Log::debug("MIDI port exit {} ({}):{}", name, ev->data.addr.client,
                 ev->data.addr.port);
      break;

    case SND_SEQ_EVENT_CLIENT_START:
      Log::debug("MIDI client start {} ({})", name, ev->data.addr.client);
      break;

    case SND_SEQ_EVENT_CLIENT_EXIT:
      // TODO Remove all client ports from maps.
      Log::debug("MIDI client exit {} ({})", name, ev->data.addr.client);
      break;

    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
      Log::debug("Port subscribed {}:{} -> {}:{}",
                 ev->data.connect.sender.client, ev->data.connect.sender.port,
                 ev->data.connect.dest.client, ev->data.connect.dest.port);
      break;

    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
      Log::debug("Port unsubscribed {}:{} -> {}:{}",
                 ev->data.connect.sender.client, ev->data.connect.sender.port,
                 ev->data.connect.dest.client, ev->data.connect.dest.port);
      break;
  }
}

void AlsaAccess::generalEvent(snd_seq_event_t *ev) noexcept {
  Log::debug("MIDI event {}:{} Ch{}", ev->source.client,
             ev->source.port, (int)ev->data.control.channel);
}

void AlsaAccess::eventLoop() noexcept {
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
      snd_seq_event_t *ev;
      err = snd_seq_event_input(_seq, &ev);
      pending = snd_seq_event_input_pending(_seq, 0) > 0;
      if (err == -ENOSPC) {
        Log::warn("MIDI event loop ENOSPC");
      } else if (err == -EAGAIN) {
        Log::warn("MIDI event loop EAGAIN");
        break;
      } else if (err < 0) {
        Log::error("MIDI event loop input: {}", strerror(-err));
        return;
      } else if (ev->source.client == _clientId &&
                 ev->source.port == _inputPortId) {
        if (ev->type == SND_SEQ_EVENT_USR0) {
          if (!internalEvent(ev)) {
            return;
          }
        } else {
          inputPortEvent(ev);
        }
      } else if (ev->source.client == SND_SEQ_CLIENT_SYSTEM &&
                 ev->source.port == SND_SEQ_PORT_SYSTEM_ANNOUNCE) {
        announcePortEvent(ev);
      } else {
        generalEvent(ev);
      }
    } while (pending);
  }
}

AlsaAccess::AlsaAccess(Options options, snd_seq_t *seq, unsigned char clientId,
                       unsigned char inputPortId)
  : Access(options),
    _seq{seq},
    _clientId{clientId},
    _inputPortId{inputPortId}
{
  collectPorts();
  _eventLoopThread = std::thread(std::bind(&AlsaAccess::eventLoop, this));
}

AlsaAccess::~AlsaAccess() {
  Log::debug("AlsaAccess::~AlsaAccess()");

  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, _inputPortId);
  snd_seq_ev_set_dest(&ev, _clientId, _inputPortId);
  snd_seq_ev_set_direct(&ev);
  ev.type = SND_SEQ_EVENT_USR0;
  memcpy(ev.data.raw8.d, ShutdownCmd, CmdLength);
  Log::debug("MIDI event loop shutdown.");
  snd_seq_event_output_direct(_seq, &ev);
  Log::debug("MIDI event thread join.");
  _eventLoopThread.join();
  Log::debug("snd_seq_close");
  snd_seq_close(_seq);
}

void AlsaAccess::create(std::promise<std::shared_ptr<Access>> promise,
                        Options options) {
  snd_seq_t *seq;
  int err;
  int clientId, inputPortId;

  auto setPromiseResult = scope_exit([&]() {
    if (err) {
      promise.set_exception(std::make_exception_ptr(
        std::system_error(-err, std::system_category(), snd_strerror(-err))
      ));
    } else {
      promise.set_value(
        std::make_shared<AlsaAccess>(options, seq, clientId, inputPortId)
      );
    }
  });

  err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
  if (err != 0) {
    Log::error("snd_seq_open failed with error {}", snd_strerror(-err));
    return;
  }

  auto closeSeq = scope_exit([=]() { snd_seq_close(seq); });

  snd_seq_set_client_name(seq, "Autumn8" /* TODO app name */);
  clientId = snd_seq_client_id(seq);

  inputPortId = snd_seq_create_simple_port(
    seq, "Input", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
    SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
  if (inputPortId < 0) {
    err = inputPortId;
    Log::error("snd_seq_create_simple_port failed: {}", snd_strerror(-err));
    return;
  }

  err = subscribeToAnnouncePort(seq, clientId, inputPortId);
  if (err != 0) {
    Log::error("Subscribe to announce port failed: {}", snd_strerror(-err));
    return;
  }

  closeSeq.release();
}
// AlsaAccess }}}

template<>
std::future<std::shared_ptr<Access>>
Access::request<Backend::AlsaSeq>(Options options) {
  std::promise<std::shared_ptr<Access>> promise;
  auto future = promise.get_future();
  AlsaAccess::create(std::move(promise), options);
  return future;
}

} // namespace a8::midi
