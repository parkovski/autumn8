#include "a8/a8.h"
#include "a8/gfx/window.h"

#include "nanovg.h"
#include "nanovg_bgfx.h"

using a8::Log;
using namespace a8::gfx;

bool Window::isValid() const {
  return !!_nativeHandle;
}

std::future<bool> Window::isValidF() const {
  // TODO: Async validation when not in main thread.
  std::promise<bool> p;
  p.set_value(!!_nativeHandle);
  return p.get_future();
}

bool Window::processEvents() {
  RawEvent e;
  bool hadEvent = false;
  while (_events.try_dequeue(e)) {
    hadEvent = true;
    std::visit([this](auto &&e) { this->process(e); }, e);
  }
  return hadEvent;
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
  bgfx::reset(e.size.width, e.size.height, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  _size = e.size;
  auto scaledSize = Size{e.size} * _scale;
  Log::trace("Sizing ({}, {})", scaledSize.width, scaledSize.height);
  _signals.sizing(*this, SizingEvent{scaledSize, e.side});
}

void Window::process(const RawMovingEvent &e) {
  if (_pos == e.pos) {
    return;
  }
  _pos = e.pos;
  auto scaledPoint = Point{e.pos} * _scale;
  Log::trace("Move ({}, {})", scaledPoint.x, scaledPoint.y);
  _signals.moving(*this, MovingEvent{scaledPoint});
}

void Window::process(const RawResizeEvent &e) {
  // if (_size == e.size) {
  //   return;
  // }
  if (_size != e.size) {
    bgfx::reset(e.size.width, e.size.height, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  }
  _size = e.size;
  auto scaledSize = Size{e.size} * _scale;
  Log::trace("Resize ({}, {})", scaledSize.width, scaledSize.height);
  _signals.resize(*this, ResizeEvent{scaledSize});
}

void Window::process(const DisplayScaleEvent &e) {
  _scale = e.scale;
  Log::trace("DisplayScale {}x ({}, {})", e.scale, e.displaySize.width, e.displaySize.height);
  _signals.displayScale(*this, e);
}

void Window::process(const SystemEvent &e) {
  _signals.system(*this, e);
  if (e == SystemEvent::Close) {
    postQuitEvent();
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

bool Window::nvgInit() {
  if (!(_nvg = nvgCreate(1, 0))) {
    return false;
  }
  nvgCreateFont(_nvg, "foo", "C:\\Users\\parker\\Documents\\GitHub\\autumn8\\res\\Roboto-Regular.ttf");
  return true;
}

void Window::nvgDestroy() {
  if (_nvg) {
    nvgDelete(_nvg);
  }
}

bool Window::render() {
  if (!processEvents()) {
    return false;
  }

  //bgfx::setViewRect(0, 0, 0, _size.width, _size.height);
  bgfx::touch(0);
  NvgRenderer renderer{_nvg};
  static_cast<Renderer &>(renderer).beginFrame(size(), scale());
  root().render(renderer);
  renderer.endFrame();

  return true;
}

View::~View() {}

View *View::root() { return &_window->root(); }

void EmptyView::render(Renderer &r) {
  r.strokeColor(Renderer::rgb(0xff, 0xff, 0xff));
  r.fillColor(Renderer::rgb(0xff, 0x80, 0x20));
  r.beginPath();
  r.circle(250, 250, 100);
  r.stroke();
  r.fill();
  auto const text = "I am some text";
  r.textBox(20, 20, 200, text);
  r.text(100, 100, "I am some text");
}
