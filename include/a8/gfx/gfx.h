#ifndef _a8_gfx_gfx_h_
#define _a8_gfx_gfx_h_

#include "concurrentqueue/concurrentqueue.h"

#include <variant>
#include <memory>
#include <string_view>
#include <cstdint>

extern "C" {
  struct NVGcontext;
}

namespace a8::gfx {

struct MouseEvent {
  enum Buttons : int {
    Button1 = 0x1,
    Button2 = 0x2,
    Button3 = 0x4,
    Button4 = 0x8,
    Button5 = 0x10,
  };

  explicit MouseEvent(float x, float y, int buttons) noexcept
    : x(x), y(y), buttons(buttons)
  {}

  bool hasButtons() const { return buttons != 0; }
  bool hasButtons(int buttons) const { return this->buttons & buttons; }

  float x;
  float y;
  int buttons;
};

struct KeyboardEvent {
  enum Modifiers : int {
    LeftShift   = 0x1,
    RightShift  = 0x2,
    Shift       = 0x3,
    LeftControl = 0x4,
    RightControl= 0x8,
    Control     = 0xC,
    LeftAlt     = 0x10,
    RightAlt    = 0x20,
    Alt         = 0x30,
    LeftWin     = 0x40,
    RightWin    = 0x80,
    Win         = 0xC0,

    KeyDown     = 0x10000,
    KeyUp       = 0x20000,
  };

  explicit KeyboardEvent(unsigned key, int modifiers) noexcept
    : key(key), modifiers(modifiers)
  {}

  bool hasModifiers(int modifiers) const { return this->modifiers & modifiers; }
  bool isDown() const { return hasModifiers(KeyDown); }
  bool isUp() const { return hasModifiers(KeyUp); }

  unsigned key;
  int modifiers;
};

struct ResizeEvent {
  enum Flags : int {
    Minimized       = 0x1,
    Maximized       = 0x2,
    FullScreen      = 0x4,
    SizeChanged     = 0x8,
    PositionChanged = 0x10,
  };

  explicit ResizeEvent(float x, float y, float width, float height, int flags) noexcept
    : x(x), y(y), width(width), height(height), flags(flags)
  {}

  bool hasFlags(int flags) const { return this->flags & flags; }
  bool didSizeChange() const { return hasFlags(SizeChanged); }
  bool didPositionChange() const { return hasFlags(PositionChanged); }

  float x;
  float y;
  float width;
  float height;
  int flags;
};

struct ScaleEvent {
  float scale;
  int dpi;

  explicit ScaleEvent(float scale, int dpi) noexcept
    : scale(scale), dpi(dpi)
  {}
};

enum class SystemEvent : int {
  Close,
  Pause,
  Resume,
  Minimize,
  Maximize,
  FullScreen,
  Restore,
  Back,
};

using Event = std::variant<
  std::monostate,
  MouseEvent,
  KeyboardEvent,
  ResizeEvent,
  ScaleEvent,
  SystemEvent
>;

struct NativeState;
class Window {
  bool processEvents();
  void process(const std::monostate &) {}
  void process(const MouseEvent &);
  void process(const KeyboardEvent &);
  void process(const ResizeEvent &);
  void process(const ScaleEvent &);
  void process(const SystemEvent &);

public:
  explicit Window(uint16_t viewId, NativeState *ns)
    : _events(4096), _viewId(viewId), _nativeState(ns) {}

  bool postEvent(Event &&event) {
    return _events.enqueue(std::move(event));
  }

  bool nvgInit();
  void nvgDestroy();

  bool render();

  uint16_t viewId() const { return _viewId; }
  NativeState *nativeState() const { return _nativeState; }

private:
  moodycamel::ConcurrentQueue<Event> _events;
  uint16_t _viewId;
  NVGcontext *_nvg;
  NativeState *_nativeState;
};

void postQuitEvent(NativeState *);

} // namespace a8::gfx

#endif // _a8_gfx_gfx_h_
