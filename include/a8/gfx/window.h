#ifndef A8_GFX_WINDOW_H_
#define A8_GFX_WINDOW_H_

#include "a8/gfx/events.h"
#include "a8/gfx/view.h"

//#include "yoga/yoga.h"

#include "concurrentqueue/concurrentqueue.h"
#include <string_view>
#include <future>
#include <array>

extern "C" {
  struct NVGcontext;
}

namespace a8::gfx {

class Window {
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

  explicit Window()
    : _events(_queueSize), _nvg(), _nativeHandle(),
      _zoom(1)
  {}

#ifdef A8_PLATFORM_WINDOWS_IMPL
  friend LRESULT CALLBACK ::a8::platform::windows::mainWndProc(
    HWND, UINT, WPARAM, LPARAM
  );
#endif

public:
  /// Disambiguation marker so that strings don't call the void* constructor.
  static inline constexpr struct native_handle_t final {} native_handle{};

  explicit Window(std::string_view title);
  explicit Window(const native_handle_t &, void *nativeHandle)
    : Window()
  {
    _nativeHandle = nativeHandle;
  }
  ~Window();

  bool isValid() const;
  std::future<bool> isValidF() const;

  bool postEvent(RawEvent &&event) {
    return _events.enqueue(std::move(event));
  }

  bool postEvents(RawEvent *begin, RawEvent *end) {
    return _events.enqueue_bulk(begin, (end - begin) / sizeof(RawEvent));
  }

  template<typename... Events>
  bool postEvents(const RawEvent &e1, const Events &...events) {
    std::array<RawEvent, 1 + sizeof...(Events)> eventsArray{
      e1, events...
    };
    return _events.enqueue_bulk(eventsArray.begin(), 1 + sizeof...(Events));
  }

  const View &root() const;
  View &root();

  bool nvgInit();
  void nvgDestroy();

  bool render();

  float zoom() const { return _zoom; }
  float zoom(float newZoom) { _zoom = newZoom; }
  float scale() const { return _scale; }
  Size size() const { return Size(_size) / _scale; }

  void *nativeHandle() const { return _nativeHandle; }

private:
  static constexpr int _queueSize = 64;
  moodycamel::ConcurrentQueue<RawEvent> _events;
  NVGcontext *_nvg;

  void *_nativeHandle;
  float _scale;
  float _zoom;
  RawPoint _pos;
  RawSize _size;
};

int runMessageLoop(Window &);
void postQuitEvent(int exitCode = 0);

} // namespace a8::gfx

#endif // A8_GFX_WINDOW_H_
