#include "a8/a8.h"
#include "alsaseq.h"

using namespace a8::midi;

AlsaOutput::AlsaOutput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                       snd_seq_port_info_t *pi)
  : Port(port_id(pi),
         "Unknown", // Manufacturer
         client_port_name(ci, pi),
         "Unknown"), // Version
    AlsaPort(seq,
             *snd_seq_port_info_get_addr(pi),
             (unsigned char)snd_seq_port_info_get_port(pi))
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

void AlsaOutput::send(const Message &message, uint32_t time) {
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
  A8_ERROR("MIDI send not implemented");
}

void AlsaOutput::clear() {
  A8_WARN("MIDI queue not implemented");
}
