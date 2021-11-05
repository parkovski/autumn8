#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/gfx/window.h"
#include "a8/util/utfconv.h"

using namespace a8::gfx;
using namespace a8::platform::Windows;

//namespace {
  // To not lose precision by storing coordinates as int.
  //constexpr int InitialCoordMultiplier = 60;
//}

Window::~Window() {}

bool Window::open(std::string_view title, void *parent) noexcept {
  DWORD style = parent
    ? WS_CHILD | WS_VISIBLE
    : WS_OVERLAPPEDWINDOW;

  auto u16title = to_utf16(title);
  _nativeHandle = CreateWindowEx(
    WS_EX_ACCEPTFILES,
    A8WindowClass,
    u16title.c_str(),
    style,
    _pos.x,
    _pos.y,
    _size.width,
    _size.height,
    (HWND)parent,
    nullptr,
    g_platformInit.hInstance,
    this
  );

  if (!_nativeHandle) {
    return false;
  }

  if (!parent) {
    ShowWindow(_nativeHandle, g_platformInit.nCmdShow);
    UpdateWindow(_nativeHandle);
  }
  return true;
}

void Window::close() noexcept {
  PostMessage(_nativeHandle, WM_USER, 0, 0);
}

void Window::show(bool show) noexcept {
  if (show) {
    ShowWindow(_nativeHandle, SW_SHOW);
    UpdateWindow(_nativeHandle);
  } else {
    ShowWindow(_nativeHandle, SW_HIDE);
  }
}

void Window::zoom(float newZoom) {
  _zoom = newZoom;
}

void Window::size(Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  if (_size == rawNewSize) {
    return;
  }
  _size = rawNewSize;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(_nativeHandle, nullptr, 0, 0, _size.width, _size.height,
               SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOZORDER
                | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

void Window::position(Point newPosition) {
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_pos == rawNewPosition) {
    return;
  }
  _pos = rawNewPosition;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(_nativeHandle, nullptr, _pos.x, _pos.y, 0, 0,
               SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER
                | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
}

void Window::resize(Point newPosition, Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_size == rawNewSize && _pos == rawNewPosition) {
    return;
  }
  _size = rawNewSize;
  _pos = rawNewPosition;
  if (!_nativeHandle) {
    return;
  }
  SetWindowPos(
    _nativeHandle, nullptr, _pos.x, _pos.y, _size.width, _size.height,
    SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
  );
}

void Window::runOnMessageThread(void (*f)(void *data), void *data) {
  void *msgdata[2] = {(void *)f, data};
  SendMessage(_nativeHandle, WM_USER, A8M_EXECONMSGTHREAD, (LPARAM)msgdata);
}
