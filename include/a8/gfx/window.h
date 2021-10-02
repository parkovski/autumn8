#ifndef A8_GFX_WINDOW_H_
#define A8_GFX_WINDOW_H_

#include "events.h"

#include "concurrentqueue/blockingconcurrentqueue.h"

#include <boost/signals2.hpp>

#include <string_view>
#include <array>
#include <vector>
#include <thread>

#if A8_IS_GFX_IMPL(Win32)
// Message types for WM_USER.

constexpr WPARAM A8M_ASYNCDESTROY = 0;
constexpr WPARAM A8M_EXECONMSGTHREAD = 1;
#endif

namespace a8::gfx {

class View;

class Window {
  static constexpr int QueueSize = 64;

public:
  template<class E>
  using Signal = ::a8::gfx::Signal<Window, E>;

  template<class E>
  using Slot = ::a8::gfx::Slot<Window, E>;

  using native_handle_t =
#if A8_IS_GFX_IMPL(Win32)
    HWND
#else
    void *
#endif
    ;

  Window() = default;

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  ~Window();

  bool open(std::string_view title, void *parent = nullptr) noexcept;
  void close() noexcept;

  void show(bool show) noexcept;

  View *setView(View *view);
  View *view() noexcept {
    return _view;
  }
  const View *view() const noexcept {
    return _view;
  }

  float zoom() const noexcept { return _zoom; }
  void zoom(float newZoom);

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

  // Waits for the function to return!
  void runOnMessageThread(void (*f)(void *data), void *data);

  // Waits for the function to return!
  template<class F>
  void runOnMessageThread(F &&f) {
    runOnMessageThread([](void *data){ (*static_cast<F *>(data))(); }, &f);
  }

  /// @type E The event type.
  /// @param slot The callback function.
  template<class E>
  auto connect(Slot<E> slot) {
    return _signals.get<E>().connect(std::move(slot));
  }

  void quitOnClose(bool quit = true) noexcept {
    _quitOnClose = quit;
  }

  bool postEvent(const RawEvent &event) {
    return _events.enqueue(event);
  }

  bool postEvents(RawEvent *begin, RawEvent *end) {
    return _events.enqueue_bulk(begin, end - begin);
  }

  template<typename E1, typename... Events>
  bool postEvents(E1 &&e1, Events &&...events) {
    std::array<RawEvent, 1 + sizeof...(Events)> eventsArray{
      std::forward<E1>(e1),
      std::forward<Events>(events)...
    };
    return _events.enqueue_bulk(eventsArray.begin(), eventsArray.size());
  }

  void *nativeHandle() const noexcept { return _nativeHandle; }

private:
  bool processEvents();
  void process(const std::monostate &) {}
  void process(const EventLoopQuitEvent &);
  void process(const CallbackEvent &);
  void process(const SystemEvent &);
  void process(const RawPointerEvent &);
  void process(const KeyboardEvent &);
  void process(const GamepadEvent &);
  void process(const MidiEvent &);
  void process(const RawSizingEvent &);
  void process(const RawMovingEvent &);
  void process(const RawResizeEvent &);
  void process(const DisplayScaleEvent &);
  void process(const StyleChangeEvent &);
  void process(const DragDropEvent &);
  void process(const DataEvent &);

#if A8_IS_GFX_IMPL(Win32)
  friend LRESULT CALLBACK ::a8::platform::windows::mainWndProc(
    HWND, UINT, WPARAM, LPARAM
  );
#endif

  moodycamel::BlockingConcurrentQueue<RawEvent> _events;
  EventSignals<Signal> _signals;
  std::thread _eventThread;

  native_handle_t _nativeHandle = nullptr;
  float _scale = 1.f;
  float _zoom = 1.f;
  RawPoint _pos{};
  RawSize _size{};
  View *_view = nullptr;
  bool _quitOnClose = false;
};

} // namespace a8::gfx

#endif // A8_GFX_WINDOW_H_
