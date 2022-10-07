#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/window/window.h"
#include "a8/util/utfconv.h"

using namespace a8::win;
using namespace a8::plat::Windows;

//namespace {
  // To not lose precision by storing coordinates as int.
  //constexpr int InitialCoordMultiplier = 60;
//}

Window::Window(native_handle_t nativeHandle) noexcept
  : _nativeHandle{nativeHandle}
{
  // TODO
  // _scale = ??
  // _pos = ??
  // _size = ??
}

Window::~Window() {}

Window *
Window::create(std::string_view title, native_handle_t parent) {
  DWORD style = parent
    ? WS_CHILD | WS_VISIBLE
    : WS_OVERLAPPEDWINDOW;

  auto u16title = to_utf16(title);
  auto nativeHandle = CreateWindowEx(
    WS_EX_ACCEPTFILES,
    A8WindowClass,
    u16title.c_str(),
    style,
    CW_USEDEFAULT,
    0,
    0,
    0,
    parent,
    nullptr,
    g_platformState.hInstance,
    this
  );

  if (!nativeHandle) {
    return nullptr;
  }

  return new Window(nativeHandle);
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

void Window::size(Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  if (_size == rawNewSize) {
    return;
  }
  _size = rawNewSize;
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
  SetWindowPos(
    _nativeHandle, nullptr, _pos.x, _pos.y, _size.width, _size.height,
    SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
  );
}

void Window::runOnMessageThread(void (*f)(void *data), void *data) {
  void *msgdata[2] = {(void *)f, data};
  SendMessage(_nativeHandle, WM_USER, A8M_EXECONMSGTHREAD, (LPARAM)msgdata);
}
