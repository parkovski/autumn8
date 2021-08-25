#ifndef A8_GFX_WINDOW_H_
#define A8_GFX_WINDOW_H_

#include "events.h"

//#include "yoga/yoga.h"

#include "concurrentqueue/blockingconcurrentqueue.h"

#include <boost/signals2.hpp>

#include <string_view>
#include <array>
#include <vector>
#include <thread>

namespace a8::gfx {

class View;

class Window {
  static constexpr int QueueSize = 64;

public:
  template<typename E>
  using winsig_t = boost::signals2::signal<bool (Window &, const E &)>;

  Window() = default;

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(Window &&) = delete;

  ~Window();

  bool open(std::string_view title) noexcept;
  void close() noexcept;

  View *setView(View *view);
  View *view() {
    return _view;
  }
  const View *view() const {
    return _view;
  }

  float zoom() const { return _zoom; }
  void zoom(float newZoom);

  float scale() const { return _scale; }

  Size size() const { return Size(_size) / _scale; }
  void size(Size newSize);
  void size(float width, float height) { size({width, height}); }

  Point position() const { return Point(_pos) / _scale; }
  void position(Point newPosition);
  void position(float x, float y) { position({x, y}); }

  void resize(Point newPosition, Size newSize);
  void resize(float x, float y, float width, float height) {
    resize({x, y}, {width, height});
  }

  /// @type E The event type.
  /// @param slot The callback function.
  template<typename E>
  auto connect(boost::signals2::slot<bool (Window &, const E &)> slot) {
    return _signals.get<E>().connect(slot);
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

  void *nativeHandle() const { return _nativeHandle; }

private:
  bool processEvents();
  void process(const std::monostate &) {}
  void process(const RawPointerEvent &);
  void process(const KeyboardEvent &);
  void process(const GamepadEvent &);
  void process(const MidiEvent &);
  void process(const RawSizingEvent &);
  void process(const RawMovingEvent &);
  void process(const RawResizeEvent &);
  void process(const DisplayScaleEvent &);
  void process(const SystemEvent &);
  void process(const StyleChangeEvent &);
  void process(const DragDropEvent &);
  void process(const DataEvent &);

#ifdef A8_PLATFORM_WINDOWS_IMPL
  friend LRESULT CALLBACK ::a8::platform::windows::mainWndProc(
    HWND, UINT, WPARAM, LPARAM
  );
#endif

  moodycamel::BlockingConcurrentQueue<RawEvent> _events;
  EventSignals<WindowEventHandler> _signals;
  std::thread _eventThread;

  void *_nativeHandle = nullptr;
  float _scale = 1.f;
  float _zoom = 1.f;
  RawPoint _pos{};
  RawSize _size{};
  View *_view = nullptr;
};

int runMessageLoop();
int runConstantFPSMessageLoop(void (*callback)(), int fps = 30);

} // namespace a8::gfx

#endif // A8_GFX_WINDOW_H_
