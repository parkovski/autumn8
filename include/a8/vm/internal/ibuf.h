#ifndef A8_VM_INTERNAL_IBUF_H_
#define A8_VM_INTERNAL_IBUF_H_

#include "../ibuf.h"
#include "../stdsym.h"

#include <vector>
#include <memory>

struct a8_vm_ibuf {
  /// @param pageSize Page size in _bytes_.
  explicit a8_vm_ibuf(a8_vm_usize pageSize = 0x1000 /* 4kB */);

  void write(uint16_t *insn, a8_vm_usize count);

  void write(uint16_t insn) {
    write(&insn, 1);
  }

private:
  void addPage() {
    _pages.emplace_back(std::make_unique<uint16_t[]>(_pageSize));
  }

  std::unique_ptr<uint16_t[]> _workBuf;
  std::vector<std::unique_ptr<uint16_t[]>> _pages;

  // Size of work buffer in uint16_t's.
  constexpr static a8_vm_usize WorkSize = 0x40;
  a8_vm_usize _workOffset;

  // Page size in uint16_t's.
  a8_vm_usize _pageSize;
  a8_vm_usize _pageOffset;
};

#endif // A8_VM_INTERNAL_IBUF_H_
