#ifndef a8_vm_stack_h_
#define a8_vm_stack_h_

#include "a8/vm/vm.h"
#include <memory>

const a8_vm_usize A8_VM_DEFAULT_STACK_SIZE =
    sizeof(a8_vm_usize) == 8 ? 0xF6000 // 984kB
  : sizeof(a8_vm_usize) == 4 ? 0x7B000 // 492kB
  : sizeof(a8_vm_usize) == 2 ? 0x3D800 // 246kB
  : 0;
// Stack grows from the top down, like in hardware.
struct a8_vm_stack {
  std::unique_ptr<char[]> _data;
  a8_vm_usize _stksize;
  a8_vm_usize _stkoff;

  explicit a8_vm_stack(a8_vm_usize stksize = A8_VM_DEFAULT_STACK_SIZE)
    : _data{std::make_unique<char[]>(stksize)},
      _stksize{stksize},
      _stkoff{stksize}
  {}

  char *pointer() const noexcept {
    return _data.get() + _stkoff;
  }

  char *pointer(a8_vm_usize offset) const noexcept {
    return _data.get() + offset;
  }

  template<class T>
  T *pointer() const noexcept {
    return std::launder(reinterpret_cast<T *>(pointer()));
  }

  template<class T>
  T *pointer(a8_vm_usize offset) const noexcept {
    return std::launder(reinterpret_cast<T *>(pointer(offset)));
  }

  a8_vm_usize offset() const noexcept {
    return _stkoff;
  }

  // Push the current offset, aligning the stack if necessary.
  // TODO: Add alignment parameter.
  void pushoff() noexcept {
    auto ofs = offset();
    reserve<a8_vm_usize>();
    *pointer<a8_vm_usize>() = ofs;
  }

  /// @param delta The number of bytes on the stack in front of the stored
  /// offset; the number of bytes reserved since `pushoff` was called.
  void popoff(a8_vm_usize delta) noexcept {
    restore(*pointer<a8_vm_usize>(offset() + delta));
  }

  void restore(a8_vm_usize ofs) noexcept {
    _stkoff = ofs;
  }

  /// On underflow, returns false and does not reserve any space.
  bool reserve(a8_vm_usize size, a8_vm_usize align = 1) noexcept {
    if (size > _stkoff) {
      // UNDERFLOW
      return false;
    }
    // Alignment can't underflow because it is an 'and' operation - 0 is
    // aligned to every size.
    _stkoff = a8_vm_align_floor(a8_vm_usize, _stkoff - size, align);
    return true;
  }

  template<class T>
  bool reserve() noexcept {
    return reserve(sizeof(T), alignof(T));
  }

  // On overflow, resets the offset and returns false.
  bool release(a8_vm_usize size) noexcept {
    if (size > _stksize - _stkoff) {
      // OVERFLOW
      _stkoff = _stksize;
      return false;
    }
    _stkoff += size;
    return true;
  }
};

#endif // a8_vm_stack_h_
