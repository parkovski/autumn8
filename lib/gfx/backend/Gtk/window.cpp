#define A8_INCLUDE_GFX_BACKEND
#include "a8/a8.h"

#include "a8/gfx/window.h"

using namespace a8::gfx;

void Window::initialize(native_handle_t nativeHandle) noexcept {
  _scale = (float)gtk_widget_get_scale_factor(nativeHandle);
  gint x, y, width, height;
  gtk_window_get_position(GTK_WINDOW(*nativeHandle), &x, &y);
  gtk_window_get_size(GTK_WINDOW(*nativeHandle), &width, &height);
  void (*sigDestroy)(GtkWidget *, gpointer) = [](GtkWidget *, gpointer data) {
    delete static_cast<Window *>(data);
  };
  g_signal_connect(G_OBJECT(*nativeHandle), "destroy",
                   G_CALLBACK(sigDestroy), this);

  initializationFinished(nativeHandle);
}

Window::Window(native_handle_t nativeHandle) noexcept
  : View(nullptr)
{
  initialize(nativeHandle);
}

Window::Window(cstring_view title)
  : View(nullptr)
{
  auto gtkwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  if (!gtkwin) {
    // TODO throw something
  }
  if (!title.empty()) {
    gtk_window_set_title(GTK_WINDOW(gtkwin), title.c_str());
  }
  initialize(gtkwin);
}

Window::~Window() {
  if (_view) {
    delete _view;
  }

  if (_quitOnClose) {
    gtk_main_quit();
    // TODO: Gtk4?
  }
}

void Window::close() noexcept {
  gtk_widget_destroy(nativeHandle());
}

void Window::show(bool show) noexcept {
  if (show) {
    gtk_widget_show_all(nativeHandle());
  } else {
    gtk_widget_hide(nativeHandle());
  }
}

void Window::size(Size newSize) {
  auto rawNewSize = RawSize(newSize * _scale);
  if (_size == rawNewSize) {
    return;
  }
  _size = rawNewSize;
  gtk_window_set_default_size(GTK_WINDOW(*nativeHandle()),
                              _size.width, _size.height);
}

void Window::position(Point newPosition) {
  auto rawNewPosition = RawPoint(newPosition * _scale);
  if (_pos == rawNewPosition) {
    return;
  }
  _pos = rawNewPosition;
  gtk_window_move(GTK_WINDOW(*nativeHandle()), _pos.x, _pos.y);
}

void Window::resize(Point newPosition, Size newSize) {
  position(newPosition);
  size(newSize);
}

void Window::childCreated(View *child) {
  // TODO if (_view) throw...
  _view = child;
}

void Window::childInitialized(View *child) {
  if (auto childHandle = *child->nativeHandle()) {
#if A8_GTK_VERSION == 3
    gtk_container_add(GTK_CONTAINER(*nativeHandle()), childHandle);
#elif A8_GTK_VERSION == 4
    gtk_window_set_child(GTK_WINDOW(*nativeHandle()), childHandle);
#endif
  }
}
