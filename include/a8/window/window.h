#ifndef A8_WINDOW_WINDOW_H_
#define A8_WINDOW_WINDOW_H_

#include "view.h"
#include "events.h"
#include "a8/util/cstring_view.h"

#if A8_IS_GFX_IMPL(Win32)
  // Message types for WM_USER.
  constexpr WPARAM A8M_ASYNCDESTROY = 0;
  constexpr WPARAM A8M_EXECONMSGTHREAD = 1;
#endif

namespace a8::win {

class Window : public View {
  void initialize(native_handle_t nativeHandle) noexcept;

public:
  template<class E>
  using Signal = ::a8::win::Signal<Window, E>;

  template<class E>
  using Slot = ::a8::win::Slot<Window, E>;

  explicit Window(native_handle_t nativeHandle) noexcept;

  explicit Window(cstring_view title);

  ~Window();

  void close() noexcept;

  void show(bool show = true) noexcept;

  View *view() noexcept {
    return _view;
  }
  const View *view() const noexcept {
    return _view;
  }

  float scale() const noexcept { return _scale; }

  Size size() const noexcept { return Size(_size) / _scale; }
  void size(Size newSize);
  void size(float width, float height) { size({width, height}); }

  Point position() const noexcept { return Point(_pos) / _scale; }
  void position(Point newPosition);
  void position(float x, float y) { position({x, y}); }

  void resize(Point newPosition, Size newSize);
  void resize(float x, float y, float width, float height) {
    resize({x, y}, {width, height});
  }

  void quitOnClose(bool quit = true) noexcept {
    _quitOnClose = quit;
  }

protected:
  void childCreated(View *child) override;
  void childInitialized(View *child) override;

private:
#if A8_IS_GFX_IMPL(Win32)
  friend LRESULT CALLBACK ::a8::plat::Windows::mainWndProc(
    HWND, UINT, WPARAM, LPARAM
  );
#endif

  float _scale = 1.f;
  RawPoint _pos{};
  RawSize _size{};
  View *_view = nullptr;
  bool _quitOnClose = false;

public:
  Signal<WindowEvent> ewindow;
  Signal<PointerEvent> epointer;
  Signal<KeyboardEvent> ekeyboard;
  Signal<SizingEvent> esizing;
  Signal<MovingEvent> emoving;
  Signal<ResizeEvent> eresize;
};

} // namespace a8::win

#endif // A8_WINDOW_WINDOW_H_
