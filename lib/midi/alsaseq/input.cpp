#include "alsaseq.h"

using namespace a8::midi;

AlsaInput::AlsaInput(snd_seq_t *seq, snd_seq_client_info_t *ci,
                     snd_seq_port_info_t *pi)
  : Port(port_id(pi),
         "Unknown", // Manufacturer
         client_port_name(ci, pi),
         "Unknown"), // Version
    AlsaPort(seq,
             *snd_seq_port_info_get_addr(pi),
             (unsigned char)snd_seq_port_info_get_port(pi))
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
