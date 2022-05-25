#include "a8/a8.h"
#include "alsaseq.h"

#include <sstream>

using namespace a8::midi;

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
  constexpr int PortCapInput =
    SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
  constexpr int PortCapOutput =
    SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;

  int portId = snd_seq_port_info_get_port(pi);
  unsigned cap = snd_seq_port_info_get_capability(pi);
  const char *name = snd_seq_port_info_get_name(pi);
  unsigned type = snd_seq_port_info_get_type(pi);
  const char *clientName = snd_seq_client_info_get_name(ci);
  int clientId = snd_seq_client_info_get_client(ci);

  if (clientId == _clientId) {
    A8_TRACE("Skip own MIDI port {}: {} ({}:{}) [{}; {}]", clientName,
             name, clientId, portId, AlsaPortCaps{cap}, AlsaPortType{type});
    input = output = false;
    return;
  }
  if (!(type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)) {
    A8_TRACE("Skip non-MIDI_GENERIC port {}: {} ({}:{}) [{}; {}]",
             clientName, name, clientId, portId, AlsaPortCaps{cap},
             AlsaPortType{type});
    input = output = false;
    return;
  }
  if (!(cap & (PortCapInput | PortCapOutput))) {
    A8_TRACE("Skip non-read/write MIDI port {}: {} ({}:{}) [{}; {}]",
             clientName, name, clientId, portId, AlsaPortCaps{cap},
             AlsaPortType{type});
    input = output = false;
    return;
  }

  input = (cap & PortCapInput) != 0;
  output = (cap & PortCapOutput) != 0;

  if (input | output) {
    A8_TRACE("MIDI {} port {}: {} ({}:{}) [{}; {}]",
             input ? output ? "in/out" : "in" : "out", clientName, name,
             clientId, portId, AlsaPortCaps{cap}, AlsaPortType{type});
  }
}

bool AlsaAccess::internalEvent(snd_seq_event_t *ev) noexcept {
  if (!memcmp(ev->data.raw8.d, ShutdownCmd, CmdLength)) {
    A8_TRACE("MIDI event loop shutdown");
    return false;
  } else {
    char data[CmdLength + 1];
    memcpy(data, ev->data.raw8.d, CmdLength);
    data[CmdLength] = 0;
    A8_WARN("Unknown internal MIDI event '{}'.", data);
  }
  return true;
}

void AlsaAccess::inputPortEvent(snd_seq_event_t *ev) noexcept {
  A8_DEBUG("Input port event {}", ev->type);
}

void AlsaAccess::announcePortEvent(snd_seq_event_t *ev) noexcept {
  snd_seq_client_info_t *ci;
  snd_seq_client_info_alloca(&ci);
  snd_seq_get_any_client_info(_seq, ev->data.addr.client, ci);
  auto name = snd_seq_client_info_get_name(ci);

  switch (ev->type) {
    case SND_SEQ_EVENT_PORT_START: {
      A8_DEBUG("MIDI port start {} ({}):{}", name, ev->data.addr.client,
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
      A8_DEBUG("MIDI port exit {} ({}):{}", name, ev->data.addr.client,
               ev->data.addr.port);
      break;

    case SND_SEQ_EVENT_CLIENT_START:
      A8_DEBUG("MIDI client start {} ({})", name, ev->data.addr.client);
      break;

    case SND_SEQ_EVENT_CLIENT_EXIT:
      // TODO Remove all client ports from maps.
      A8_DEBUG("MIDI client exit {} ({})", name, ev->data.addr.client);
      break;

    case SND_SEQ_EVENT_PORT_SUBSCRIBED:
      A8_DEBUG("Port subscribed {}:{} -> {}:{}",
               ev->data.connect.sender.client, ev->data.connect.sender.port,
               ev->data.connect.dest.client, ev->data.connect.dest.port);
      break;

    case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
      A8_DEBUG("Port unsubscribed {}:{} -> {}:{}",
               ev->data.connect.sender.client, ev->data.connect.sender.port,
               ev->data.connect.dest.client, ev->data.connect.dest.port);
      break;
  }
}

void AlsaAccess::generalEvent(snd_seq_event_t *ev) noexcept {
  std::shared_ptr<Input> port;
  std::string portId = (
    std::stringstream{}
      << (int)ev->source.client
      << ':'
      << (int)ev->source.port
  ).str();

  {
    auto [lock, inputs] = inputsRead();
    auto it = inputs.find(portId);
    if (it == inputs.end()) {
      A8_TRACE("Ignore input port event {} for {}", AlsaEventType{ev->type},
               portId);
      return;
    }
    port = it->second;
  }

  A8_DEBUG("Input port event {} for {} ({})", AlsaEventType{ev->type},
           port->name(), portId);

  std::static_pointer_cast<AlsaInput>(port)
    ->emit(ev->time.tick, to_message(*ev));
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
        A8_WARN("MIDI event loop EINTR");
        continue;
      } else {
        A8_ERROR("MIDI event loop poll failed: {}", strerror(errno));
        return;
      }
    }

    bool pending;
    do {
      snd_seq_event_t *ev;
      err = snd_seq_event_input(_seq, &ev);
      pending = snd_seq_event_input_pending(_seq, 0) > 0;
      if (err == -ENOSPC) {
        A8_WARN("MIDI event loop ENOSPC");
      } else if (err == -EAGAIN) {
        A8_WARN("MIDI event loop EAGAIN");
        break;
      } else if (err < 0) {
        A8_ERROR("MIDI event loop: {}", strerror(-err));
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
  A8_DEBUG("AlsaAccess::~AlsaAccess()");

  snd_seq_event_t ev;
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, _inputPortId);
  snd_seq_ev_set_dest(&ev, _clientId, _inputPortId);
  snd_seq_ev_set_direct(&ev);
  ev.type = SND_SEQ_EVENT_USR0;
  memcpy(ev.data.raw8.d, ShutdownCmd, CmdLength);
  A8_DEBUG("MIDI event loop shutdown.");
  snd_seq_event_output_direct(_seq, &ev);
  A8_DEBUG("MIDI event thread join.");
  _eventLoopThread.join();
  A8_DEBUG("snd_seq_close");
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
    A8_ERROR("snd_seq_open failed with error {}", snd_strerror(-err));
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
    A8_ERROR("snd_seq_create_simple_port failed: {}", snd_strerror(-err));
    return;
  }

  err = subscribeToAnnouncePort(seq, clientId, inputPortId);
  if (err != 0) {
    A8_ERROR("Subscribe to announce port failed: {}", snd_strerror(-err));
    return;
  }

  closeSeq.release();
}
