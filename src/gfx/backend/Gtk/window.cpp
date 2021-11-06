#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/gfx/window.h"

using namespace a8::gfx;
using namespace a8::platform::Linux;

Window::~Window() {}

bool Window::open(std::string_view title, void *parent) noexcept {
  return false;
}

void Window::close() noexcept {
}

void Window::show(bool show) noexcept {
}

void Window::zoom(float newZoom) {
}

void Window::size(Size newSize) {
}

void Window::position(Point newPosition) {
}

void Window::resize(Point newPosition, Size newSize) {
}

void Window::runOnMessageThread(void (*f)(void *data), void *data) {
}
