#include "a8/a8.h"
#include "a8/gfx/window.h"

using a8::Log;
using namespace a8::gfx;

View *Window::setView(View *view) {
  View *oldView = _view;
  _view = view;
  return oldView;
}

bool Window::processEvents() {
  while (true) {
    RawEvent e;
    _events.wait_dequeue(e);
    if (std::holds_alternative<EventLoopQuitEvent>(e)) {
      // Return true if the queue is empty, false if there are pending events
      // that will not be handled.
      return !_events.try_dequeue(e);
    }
    std::visit([this](auto &&e) { this->process(e); }, e);
  }
}

void Window::process(const EventLoopQuitEvent &) {
  A8_UNREACHABLE;
}

void Window::process(const RawPointerEvent &e) {
  auto scaledPoint = Point{e.pos} * _scale;
  _signals.pointer(*this, PointerEvent{scaledPoint, e.amount, e.params});
}

void Window::process(const KeyboardEvent &e) {
  _signals.keyboard(*this, e);
}

void Window::process(const GamepadEvent &e) {
  _signals.gamepad(*this, e);
}

void Window::process(const MidiEvent &e) {
  _signals.midi(*this, e);
}

void Window::process(const RawSizingEvent &e) {
  if (_size == e.size) {
    return;
  }
  _size = e.size;
  auto scaledSize = Size{e.size} * _scale;
  //Log::trace("Sizing ({}, {})", scaledSize.width, scaledSize.height);
  _signals.sizing(*this, SizingEvent{scaledSize, e.side});
}

void Window::process(const RawMovingEvent &e) {
  if (_pos == e.pos) {
    return;
  }
  _pos = e.pos;
  auto scaledPoint = Point{e.pos} * _scale;
  //Log::trace("Move ({}, {})", scaledPoint.x, scaledPoint.y);
  _signals.moving(*this, MovingEvent{scaledPoint});
}

void Window::process(const RawResizeEvent &e) {
  // if (_size == e.size) {
  //   return;
  // }
  _size = e.size;
  auto scaledSize = Size{e.size} * _scale;
  //Log::trace("Resize ({}, {})", scaledSize.width, scaledSize.height);
  _signals.resize(*this, ResizeEvent{scaledSize});
}

void Window::process(const DisplayScaleEvent &e) {
  _scale = e.scale;
  Log::trace("DisplayScale {}x ({}, {})", e.scale, e.displaySize.width, e.displaySize.height);
  _signals.displayScale(*this, e);
}

void Window::process(const SystemEvent &e) {
  Log::trace("System {}", e);
  if (_signals.system(*this, e)) {
    if (e == SystemEvent::Close) {
      close();
    }
  }
}

void Window::process(const StyleChangeEvent &e) {
  _signals.styleChange(*this, e);
}

void Window::process(const DragDropEvent &e) {
  _signals.dragDrop(*this, e);
}

void Window::process(const DataEvent &e) {
  _signals.data(*this, e);
}
