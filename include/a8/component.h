#pragma once

#include <cstdint>

namespace a8 {

class Component {
public:
  virtual void receiveMessage(int id, void *buffer, size_t size) = 0;
};

} // namespace a8
