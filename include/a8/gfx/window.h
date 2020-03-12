#ifndef A8_GFX_WINDOW_H_
#define A8_GFX_WINDOW_H_

#include "events.h"

//#include "yoga/yoga.h"

#include "concurrentqueue/concurrentqueue.h"

#include <boost/signals2.hpp>

#include <string_view>
#include <array>
#include <vector>

extern "C" {
  struct NVGcontext;
}

namespace a8::gfx {

class View;

class Window {
  static constexpr int QueueSize = 64;

  explicit Window() noexcept;

public:
  template<typename E>
  using winsig_t = boost::signals2::signal<bool (Window &, const E &)>;

  static Window *create(std::string_view title, Point pos = Point{},
                        Size size = Size{});

  ~Window();

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

  /// @type E The event type.
  /// @param slot The callback function.
  template<typename E>
  auto connect(boost::signals2::slot<bool (Window &, const E &)> slot) {
    return _signals.get<E>().connect(slot);
  }

  View *setView(View *view);
  View *view() {
    return _view;
  }
  const View *view() const {
    return _view;
  }

  View *activeView();
  const View *activeView() const {
    return const_cast<Window *>(this)->activeView();
  }

  View *findView(const Point &point);
  const View *findView(const Point &point) const {
    return const_cast<Window *>(this)->findView(point);
  }

  bool nvgInit();
  void nvgDestroy();

  bool render();

  float zoom() const { return _zoom; }
  void zoom(float newZoom) { _zoom = newZoom; }

  float scale() const { return _scale; }

  Size size() const { return Size(_size) / _scale; }
  void size(Size newSize);

  Point position() const { return Point(_pos) / _scale; }
  void position(Point newPosition);

  void resize(Point newPosition, Size newSize);

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

  moodycamel::ConcurrentQueue<RawEvent> _events;
  EventSignals<WindowEventHandler> _signals;
  NVGcontext *_nvg;

  void *_nativeHandle;
  float _scale;
  float _zoom;
  RawPoint _pos;
  RawSize _size;
  View *_view;
};

int runMessageLoop(Window &);
void postQuitEvent(int exitCode = 0);

} // namespace a8::gfx

#endif // A8_GFX_WINDOW_H_
