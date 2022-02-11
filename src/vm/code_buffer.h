#ifndef a8_vm_code_buffer_h_
#define a8_vm_code_buffer_h_

#include "a8/vm/vm.h"

#include <vector>
#include <memory>

struct a8_vm_code_buffer {
  a8_vm_usize _pageSize;
  std::vector<std::unique_ptr<char[]>> _pages;

  a8_vm_code_buffer(a8_vm_usize pageSize = 0x1000 /* 4kB */)
    : _pageSize{pageSize}
  {}
};

#endif // a8_vm_code_buffer_h_
