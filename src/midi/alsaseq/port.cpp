#include "alsaseq.h"

#include <sstream>

using namespace a8::midi;

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
