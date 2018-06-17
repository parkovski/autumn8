#include "a8/a8.h"
#include "a8/gfx/window.h"

#include "nanovg.h"
#include "nanovg_bgfx.h"

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

}

void Window::process(const KeyboardEvent &e) {

}

void Window::process(const GamepadEvent &e) {

}

void Window::process(const MidiEvent &e) {

}

void Window::process(const RawSizingEvent &e) {
  _size = e.size;
  bgfx::reset(e.size.width, e.size.height, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  logtrace("Size {}x{}", e.size.width, e.size.height);
  //bgfx::setViewRect(0, 0, 0, (uint16_t)e.size.x, (uint16_t)e.size.y);
}

void Window::process(const RawMovingEvent &e) {
}

void Window::process(const RawResizeEvent &e) {
  _size = e.size;
  bgfx::reset(e.size.width, e.size.height, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  //bgfx::setViewRect(0, 0, 0, (uint16_t)e.size.x, (uint16_t)e.size.y);
}

void Window::process(const DisplayScaleEvent &e) {
  //bgfx::reset(e.displayWidth, e.displayHeight, BGFX_RESET_VSYNC | BGFX_RESET_HIDPI);
  _scale = e.scale;
  logtrace("Monitor {}x ({}, {})", e.scale, e.displaySize.width, e.displaySize.height);
}

void Window::process(const SystemEvent &e) {
  if (e == SystemEvent::Close) {
    postQuitEvent();
  }
}

void Window::process(const StyleChangeEvent &e) {

}

void Window::process(const DragDropEvent &e) {

}

void Window::process(const DataEvent &e) {

}

bool Window::nvgInit() {
  return (_nvg = nvgCreate(1, 0));
}

void Window::nvgDestroy() {
  nvgDelete(_nvg);
}

bool Window::render() {
  if (!processEvents()) {
    return false;
  }

  // TODO: What to pass here?
  bgfx::touch(0);
  nvgBeginFrame(_nvg, _size.width / _scale, _size.height / _scale, _scale);
  nvgStrokeColor(_nvg, nvgRGB(0xff, 0xff, 0xff));
  nvgFillColor(_nvg, nvgRGB(0xff, 0x80, 0x20));
  nvgBeginPath(_nvg);
  nvgCircle(_nvg, 250, 250, 100);
  nvgStroke(_nvg);
  nvgFill(_nvg);

  const char text[] = "I am some text. My name is jorj and I am a usb drive or something. A microSD card I guess.";
  const char *end = text + sizeof(text);

  nvgCreateFont(_nvg, "foo", "D:\\dev\\nanovg\\example\\Roboto-Regular.ttf");
  nvgTextBox(_nvg, 20, 20, 200, text, end);
  nvgText(_nvg, 100, 100, text, end);
  nvgEndFrame(_nvg);
  return true;
}
