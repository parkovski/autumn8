#ifndef A8_GFX_EVENTS_H_
#define A8_GFX_EVENTS_H_

#include "geometry.h"

#include <variant>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <boost/signals2.hpp>

namespace a8::gfx {

enum class PointerKind {
  Unknown = 0,
  Mouse   = 1,
  Touch   = 2,
  Pen     = 3,
  XY      = 4,
};

inline std::ostream &operator<<(std::ostream &os, PointerKind pk) {
  switch (pk) {
    default:
      os << "Unknown";
      break;
    case PointerKind::Mouse:
      os << "Mouse";
      break;
    case PointerKind::Touch:
      os << "Touch";
      break;
    case PointerKind::Pen:
      os << "Pen";
      break;
    case PointerKind::XY:
      os << "XY";
      break;
  }

  return os;
}

enum class PointerAction {
  /// The pointer was pressed (tapped, clicked, etc) on the active view.
  Pressed   = 0x10,
  /// The pointer was released on the active view.
  Released  = 0x20,
  /// The pointer was held down for the hold threshold time.
  Held      = 0x30,
  /// The pointer was moved over the view.
  Moved     = 0x40,
  /// The pointer entered the view's screen area.
  Entered   = 0x50,
  /// The pointer exited the view's screen area.
  Exited    = 0x60,
  /// The pointer stayed still over the view for the hold threshold time.
  Hovered   = 0x70,
  /// The pointer was moved over the view while held down.
  Dragged   = 0x80,
  /// The mouse was scrolled vertically.
  VScrolled = 0x90,
  /// The mouse was scrolled horizontally.
  HScrolled = 0xA0,
};

inline std::ostream &operator<<(std::ostream &os, PointerAction pa) {
  switch (pa) {
    default:
      os << "Unknown";
      break;
    case PointerAction::Pressed:
      os << "Pressed";
      break;
    case PointerAction::Released:
      os << "Released";
      break;
    case PointerAction::Held:
      os << "Held";
      break;
    case PointerAction::Moved:
      os << "Moved";
      break;
    case PointerAction::Entered:
      os << "Entered";
      break;
    case PointerAction::Exited:
      os << "Exited";
      break;
    case PointerAction::Hovered:
      os << "Hovered";
      break;
    case PointerAction::Dragged:
      os << "Dragged";
      break;
    case PointerAction::VScrolled:
      os << "VScrolled";
      break;
    case PointerAction::HScrolled:
      os << "HScrolled";
      break;
  }

  return os;
}

template<bool scaled>
struct PointerEventS {
private:
  static constexpr int PointerKindMask  =    0x7;
  static constexpr int ActionMask       =   0xF0;
  static constexpr int IsPressedMask    =  0x100;
  static constexpr int IndexMask        = 0x1E00;
  static constexpr int IndexShift       = 9;
  static constexpr int RepeatMask       = 0xE000;
  static constexpr int RepeatShift      = 13;

  static int32_t
  pack(PointerKind pointer, PointerAction action, bool pressed, int index, int repeat) {
    return
      static_cast<int32_t>(pointer) |
      static_cast<int32_t>(action) |
      (pressed ? IsPressedMask : 0) |
      (index << IndexShift) |
      (repeat << RepeatShift);
  }

public:
  static constexpr int MaxIndex = 15;
  static constexpr int MaxRepeat = 7;

  explicit constexpr PointerEventS(PointS<scaled> point, float amount, int32_t params) noexcept
    : pos(point), amount(amount), params(params)
  {}

  explicit constexpr PointerEventS(
    PointS<scaled> point, float amount,
    PointerKind pointer, PointerAction action, bool pressed, int index, int repeat
  ) noexcept
    : pos(point), amount(amount),
      params(pack(pointer, action, pressed, index, repeat))
  {
    assert(index >= 0 && index <= MaxIndex);
    assert(repeat >= 0 && repeat <= MaxRepeat);
  }

  explicit constexpr PointerEventS(const PointerEventS<!scaled> e, float scale) noexcept
    : PointerEventS(PointS<!scaled>(e.pos, scale), e.amount, e.params)
  {}

  PointerEventS(const PointerEventS<scaled> &) = default;
  PointerEventS<scaled> &operator=(const PointerEventS<scaled> &) = default;

  /// What kind of pointer device was used?
  constexpr PointerKind pointerKind() const {
    return static_cast<PointerKind>(params & PointerKindMask);
  }

  /// Which action was performed with the pointer?
  constexpr PointerAction action() const {
    return static_cast<PointerAction>(params & ActionMask);
  }

  /// Was the pointer in the pressed state for this action?
  constexpr bool isPressed() const {
    return params & IsPressedMask;
  }

  /// Which mouse button or touch point was used?
  constexpr int pointerIndex() const {
    return (params & IndexMask) >> IndexShift;
  }

  /// What is the repeat count for this event?
  constexpr int repeat() const {
    return (params & RepeatMask) >> RepeatShift;
  }

  /// Coordinates relative to the current view.
  PointS<scaled> pos;
  /// Pen/touch pressure, scroll delta, hold/drag time.
  float amount;
  /// Packed parameters; prefer getter functions.
  int32_t params;
};

using RawPointerEvent = PointerEventS<false>;
using PointerEvent = PointerEventS<true>;

struct KeyboardEvent {
  enum Modifiers : uint32_t {
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
#ifdef __APPLE__
    LeftCtrlCmd = LeftWin,
    RightCtrlCmd= RightWin,
    CtrlCmd     = Win,
#else
    LeftCtrlCmd = LeftControl,
    RightCtrlCmd= RightControl,
    CtrlCmd     = Control,
#endif

    KeyDown     = 0x1000,
    KeyUp       = 0x2000,
    KeyHeld     = 0x4000,
  };

  struct ShowModifiers {
    uint32_t modifiers;
  };

  explicit constexpr KeyboardEvent(uint32_t key, uint32_t modifiers) noexcept
    : key(key), modifiers(modifiers)
  {}

  KeyboardEvent(const KeyboardEvent &) = default;
  KeyboardEvent &operator=(const KeyboardEvent &) = default;

  constexpr bool hasModifiers(uint32_t modifiers) const {
    return this->modifiers & modifiers;
  }
  constexpr bool isDown() const { return hasModifiers(KeyDown); }
  constexpr bool isUp() const { return hasModifiers(KeyUp); }
  constexpr bool isHeld() const { return hasModifiers(KeyHeld); }
  constexpr ShowModifiers showModifiers() const { return {modifiers}; }

  uint32_t key;
  uint32_t modifiers;
};

inline std::ostream &
operator<<(std::ostream &os, KeyboardEvent::ShowModifiers ms) {
  auto m = ms.modifiers;
  if (m & KeyboardEvent::KeyHeld) {
    os << "Held";
  } else if (m & KeyboardEvent::KeyDown) {
    os << "Down";
  } else if (m & KeyboardEvent::KeyUp) {
    os << "Up";
  }

  if (m & KeyboardEvent::LeftShift) {
    os << " LShift";
  }
  if (m & KeyboardEvent::RightShift) {
    os << " RShift";
  }
  if (m & KeyboardEvent::LeftControl) {
    os << " LCtrl";
  }
  if (m & KeyboardEvent::RightControl) {
    os << " RCtrl";
  }
  if (m & KeyboardEvent::LeftAlt) {
    os << " LAlt";
  }
  if (m & KeyboardEvent::RightAlt) {
    os << " RAlt";
  }
  if (m & KeyboardEvent::LeftWin) {
    os << " LWin";
  }
  if (m & KeyboardEvent::RightWin) {
    os << " RWin";
  }

  return os;
}

struct GamepadEvent {
  enum Control {
    Button1 = 1,
  };

  explicit constexpr GamepadEvent(Control control, float xAmount, float yAmount) noexcept
    : control(control), xAmount(xAmount), yAmount(yAmount)
  {}

  explicit constexpr GamepadEvent(Control control, float amount) noexcept
    : control(control), xAmount(amount), yAmount(amount)
  {}

  GamepadEvent(const GamepadEvent &) = default;
  GamepadEvent &operator=(const GamepadEvent &) = default;

  Control control;
  float xAmount;
  float yAmount;
};

struct MidiEvent {
  explicit constexpr MidiEvent(
    uint8_t port, uint8_t channel, uint8_t note, uint8_t velocity
  ) noexcept
    : port(port), channel(channel), note(note), velocity(velocity)
  {}

  MidiEvent(const MidiEvent &) = default;
  MidiEvent &operator=(const MidiEvent &) = default;

  uint8_t port;
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
};

template<bool scaled>
struct SizingEventS {
  explicit constexpr SizingEventS(SizeS<scaled> size, Anchor side) noexcept
    : size(size), side(side)
  {
    assert(size.width >= 0 && size.height >= 0);
  }

  explicit constexpr SizingEventS(const SizingEventS<!scaled> &e, float scale) noexcept
    : SizingEventS(SizeS<!scaled>(e.size, scale), e.side)
  {}

  SizingEventS(const SizingEventS<scaled> &) = default;
  SizingEventS<scaled> &operator=(const SizingEventS<scaled> &) = default;

  SizeS<scaled> size;
  Anchor side;
};

using RawSizingEvent = SizingEventS<false>;
using SizingEvent = SizingEventS<true>;

template<bool scaled>
struct MovingEventS {
  explicit constexpr MovingEventS(PointS<scaled> point) noexcept
    : pos(point)
  {}

  explicit constexpr MovingEventS(MovingEventS<!scaled> &e, float scale) noexcept
    : MovingEventS(PointS<!scaled>(e.pos, scale))
  {}

  MovingEventS(const MovingEventS<scaled> &) = default;
  MovingEventS<scaled> &operator=(const MovingEventS<scaled> &) = default;

  PointS<scaled> pos;
};

using RawMovingEvent = MovingEventS<false>;
using MovingEvent = MovingEventS<true>;

template<bool scaled>
struct ResizeEventS {
  explicit constexpr ResizeEventS(SizeS<scaled> size) noexcept
    : size(size)
  {
    assert(size.width >= 0 && size.height >= 0);
  }

  explicit constexpr ResizeEventS(const ResizeEventS<!scaled> &e, float scale) noexcept
    : ResizeEventS(SizeS<!scaled>(e.size, scale))
  {}

  ResizeEventS(const ResizeEventS<scaled> &) = default;
  ResizeEventS<scaled> &operator=(const ResizeEventS<scaled> &) = default;

  SizeS<scaled> size;
};

using RawResizeEvent = ResizeEventS<false>;
using ResizeEvent = ResizeEventS<true>;

struct DisplayScaleEvent {
  explicit constexpr DisplayScaleEvent(float scale, SizeT<uint32_t> displaySize) noexcept
    : scale(scale), displaySize(displaySize)
  {}

  DisplayScaleEvent(const DisplayScaleEvent &) = default;
  DisplayScaleEvent &operator=(const DisplayScaleEvent &) = default;

  float scale;
  RawSize displaySize;
};

enum class SystemEvent {
  /// Sent when the view is created.
  Initialize,
  /// Sent when the view is being destroyed.
  Destroy,
  /// Sent when the user requests a close action.
  Close,
  /// Sent when the window is minimized.
  Minimize,
  /// Sent when the window is maximized.
  Maximize,
  /// Sent when the window is unminimized.
  RestoreSize,
  /// Sent when the window is made full screen.
  FullScreen,
  /// Sent when the system pauses the app.
  Pause,
  /// Sent when the system resumes the app.
  Resume,
  /// Sent when the app is brought to the foreground.
  Focus,
  /// Sent when the app is sent to the background.
  Unfocus,
  /// Sent when the system back button is pressed.
  Back,
};

std::ostream &operator<<(std::ostream &os, SystemEvent e);

struct EventLoopQuitEvent {};

struct StyleChangeEvent {
  int todo;
};

struct DragDropEvent {
  int todo;
};

struct DataEvent {
private:
  // Helper to initialize a variable length array
  template<typename B, size_t... I>
  explicit constexpr DataEvent(
    void *pointer, uint32_t tag, const B &auxValueByte, std::index_sequence<I...>
  ) noexcept
    : pointer(pointer), tag(tag), auxValueByte{auxValueByte[I]...}
  {}

public:
  explicit constexpr DataEvent(void *pointer, uint32_t tag) noexcept
    : pointer(pointer), tag(tag), auxValueI32(0)
  {}

  explicit constexpr DataEvent(void *pointer, uint32_t tag, int32_t auxValueI32) noexcept
    : pointer(pointer), tag(tag), auxValueI32(auxValueI32)
  {}

  explicit constexpr DataEvent(void *pointer, uint32_t tag, float auxValueFloat) noexcept
    : pointer(pointer), tag(tag), auxValueFloat(auxValueFloat)
  {}

  template<typename B, size_t N, typename Index = std::make_index_sequence<N>,
    typename = std::enable_if_t<(sizeof(B) == sizeof(int8_t) && N <= 4)>
  >
  explicit constexpr DataEvent(void *pointer, uint32_t tag, B auxValueByte[N]) noexcept
    : DataEvent(pointer, tag, auxValueByte, Index{})
  {}

  DataEvent(const DataEvent &) = default;
  DataEvent &operator=(const DataEvent &) = default;

  void *pointer;
  uint32_t tag;
  union {
    int32_t auxValueI32;
    int8_t auxValueByte[4];
    float auxValueFloat;
  };
};

static_assert(sizeof(DataEvent) == 2 * sizeof(int64_t));

template<bool scaled>
using EventS = std::variant<
  std::monostate,
  EventLoopQuitEvent,
  PointerEventS<scaled>,
  KeyboardEvent,
  GamepadEvent,
  MidiEvent,
  SizingEventS<scaled>,
  MovingEventS<scaled>,
  ResizeEventS<scaled>,
  DisplayScaleEvent,
  SystemEvent,
  StyleChangeEvent,
  DragDropEvent,
  DataEvent
>;

using RawEvent = EventS<false>;
using Event = EventS<true>;

// Signals can return false to stop the event from propogating further. If no
// signals are present or all return true, a default behavior may be invoked.
struct ShortCircuit {
  using result_type = bool;

  template<class InputIterator>
  bool operator()(InputIterator first, InputIterator last) const {
    while (first != last) {
      if (!*first++) {
        return false;
      }
    }
    return true;
  }
};

template<class R, class E>
using Signal = boost::signals2::signal<bool (R &, const E &), ShortCircuit>;

template<class R, class E>
using Slot = boost::signals2::slot<bool (R &, const E &)>;

template<template<class> class EventKind>
struct EventSignals {
  // Let you refer to the signals here by type.
  template<class E> EventKind<E> &get();

  // template<typename E> const E &get() const {
  //   return const_cast<EventSignals *>(this)->get<E>();
  // }

#define DEFEVENT(Class, Name) \
  EventKind<Class> Name; \
  template<> EventKind<Class> &get<Class>() { return Name; }

  DEFEVENT(PointerEvent, pointer)
  DEFEVENT(KeyboardEvent, keyboard)
  DEFEVENT(GamepadEvent, gamepad)
  DEFEVENT(MidiEvent, midi)
  DEFEVENT(SizingEvent, sizing)
  DEFEVENT(MovingEvent, moving)
  DEFEVENT(ResizeEvent, resize)
  DEFEVENT(DisplayScaleEvent, displayScale)
  DEFEVENT(SystemEvent, system)
  DEFEVENT(StyleChangeEvent, styleChange)
  DEFEVENT(DragDropEvent, dragDrop)
  DEFEVENT(DataEvent, data)

#undef DEFEVENT
};

} // namespace a8::gfx

#endif // A8_GFX_EVENTS_H_
