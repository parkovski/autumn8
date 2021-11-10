#ifndef A8_GFX_VIEWS_VIEW_H_
#define A8_GFX_VIEWS_VIEW_H_

#include "a8/a8.h"
#include "../events.h"

namespace a8::gfx {

struct native_handle_t {
  using type =
#if A8_IS_GFX_IMPL(Win32)
    HWND
#elif A8_IS_GFX_IMPL(Gtk)
    GtkWidget *
#else
    void *
#endif
    ;

  type handle = nullptr;

  constexpr native_handle_t() noexcept = default;
  constexpr native_handle_t(type handle) noexcept
    : handle{handle}
  {}
  constexpr native_handle_t(const native_handle_t &) noexcept = default;
  constexpr native_handle_t &operator=(const native_handle_t &) noexcept = default;

  constexpr type &operator*() noexcept {
    return handle;
  }

  constexpr const type &operator*() const noexcept {
    return handle;
  }

  constexpr type *operator&() noexcept {
    return &handle;
  }

  constexpr const type *operator&() const noexcept {
    return &handle;
  }

  constexpr operator type() noexcept {
    return handle;
  }
};

constexpr bool operator==(native_handle_t a, native_handle_t b) noexcept {
  return a.handle == b.handle;
}

constexpr bool operator!=(native_handle_t a, native_handle_t b) noexcept {
  return a.handle != b.handle;
}

class View {
public:
  template<class E>
  using Signal = ::a8::gfx::Signal<View, E>;

  template<class E>
  using Slot = ::a8::gfx::Slot<View, E>;

  explicit View(View *parent) noexcept
    : _parent{parent}
  {
    if (parent) {
      parent->childCreated(this);
    }
  }

  View(const View &) = delete;
  View &operator=(const View &) = delete;

  virtual ~View();

  View *parent() noexcept { return _parent; }
  const View *parent() const noexcept { return _parent; }

  // May be null depending on the backend toolkit and view implementation.
  native_handle_t nativeHandle() const noexcept { return _nativeHandle; }

protected:
  // Child classes must call this when they have finished their initialization.
  void initializationFinished(native_handle_t nativeHandle = {}) {
    _nativeHandle = nativeHandle;
    if (_parent) {
      _parent->childInitialized(this);
    }
  }

  // Called on the parent _before_ the child is initialized. In particular,
  // a child that will have a native handle will not at this point.
  virtual void childCreated(View *child);

  // Called on the parent after the child is fully initialized.
  virtual void childInitialized(View *child);

private:
  native_handle_t _nativeHandle;
  View *_parent;
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_VIEW_H_
