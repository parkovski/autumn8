#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/gfx/window.h"

using namespace a8::gfx;
using namespace a8::platform::Linux;

Window::Window(void *nativeHandle) noexcept
  : _nativeHandle{(native_handle_t)nativeHandle}
{
  _scale = (float)gtk_widget_get_scale_factor(_nativeHandle);
  gint x, y, width, height;
  gtk_window_get_position(GTK_WINDOW(_nativeHandle), &x, &y);
  gtk_window_get_size(GTK_WINDOW(_nativeHandle), &width, &height);
  void (*sigDestroy)(GtkWidget *, gpointer) = [](GtkWidget *, gpointer data) {
    delete static_cast<Window *>(data);
  };
  g_signal_connect(G_OBJECT(_nativeHandle), "destroy", G_CALLBACK(sigDestroy),
                   this);
}

Window::~Window() {}

Window *
Window::create(std::string_view title, void *parent) {
  auto nativeHandle = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  if (!nativeHandle) {
    return nullptr;
  }
  if (parent) {
    //TODO ??
    //gtk_widget_set_parent(nativeHandle, (native_handle_t)parent);
  }
  std::string title_s{title};
  gtk_window_set_title(GTK_WINDOW(nativeHandle), title_s.c_str());
  return new Window(nativeHandle);
}

void Window::close() noexcept {
  gtk_widget_destroy(_nativeHandle);
}

void Window::show(bool show) noexcept {
  if (show) {
    gtk_widget_show_all(_nativeHandle);
  } else {
    gtk_widget_hide(_nativeHandle);
  }
}

void Window::size(Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  if (_size == rawNewSize) {
    return;
  }
  _size = rawNewSize;
  gtk_window_set_default_size(GTK_WINDOW(_nativeHandle),
                              _size.width, _size.height);
}

void Window::position(Point newPosition) {
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_pos == rawNewPosition) {
    return;
  }
  _pos = rawNewPosition;
  gtk_window_move(GTK_WINDOW(_nativeHandle), _pos.x, _pos.y);
}

void Window::resize(Point newPosition, Size newSize) {
  position(newPosition);
  size(newSize);
}

void Window::runOnMessageThread(void (*f)(void *data), void *data) {
  // TODO - g_idle_add / g_source_new / g_source_set_callback
}
