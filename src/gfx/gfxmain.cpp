#include "a8/gfx/gfx.h"

#include "nanovg.h"
#include "nanovg_bgfx.h"

using namespace a8;
using namespace a8::gfx;

bool Window::processEvents() {
  Event e;
  bool hadEvent = false;
  while (_events.try_dequeue(e)) {
    hadEvent = true;
    std::visit([this](auto &&e) { this->process(e); }, e);
  }
  return hadEvent;
}

void Window::process(const MouseEvent &e) {

}

void Window::process(const KeyboardEvent &e) {

}

void Window::process(const ResizeEvent &e) {

}

void Window::process(const ScaleEvent &e) {

}

void Window::process(const SystemEvent &e) {
  if (e == SystemEvent::Close) {
    postQuitEvent(_nativeState);
  }
}

bool Window::nvgInit() {
  return (_nvg = nvgCreate(2, _viewId));
}

void Window::nvgDestroy() {
  nvgDelete(_nvg);
}

bool Window::render() {
  if (!processEvents()) {
    //return false;
  }

  nvgStrokeColor(_nvg, nvgRGB(0xff, 0xff, 0xff));
  nvgFillColor(_nvg, nvgRGB(0xff, 0x80, 0x20));
  nvgCircle(_nvg, 10, 10, 4);
  return true;
}
