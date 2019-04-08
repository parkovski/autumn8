#ifndef A8_GFX_VIEW_H_
#define A8_GFX_VIEW_H_

#include "renderer.h"
#include <vector>
#include <memory>

namespace a8::gfx {

class Window;

class View {
  View *_parent;
  Window *_window;

public:
  View() = delete;

  explicit View(Window *window) noexcept
    : _parent{nullptr}, _window{window}
  {}

  // @param parent may not be null.
  explicit View(View *parent) noexcept
    : _parent{parent}, _window{parent->_window}
  {}

  View(const View &) = delete;
  View &operator=(const View &) = delete;

  View(View &&) = default;
  View &operator=(View &&) = default;

  virtual ~View() = 0;

  View *parent() {
    return _parent;
  }

  View *root();

  Window *window() {
    return _window;
  }

  virtual void render(Renderer &r) = 0;
};

class EmptyView : public View {
public:
  using View::View;
  void render(Renderer &) override;
};

class CanvasView : public View {
  std::function<void(View &, Renderer &)> _renderCallback;

public:
  explicit CanvasView(
    View *parent, std::function<void(View &, Renderer &)> renderCallback
  ) noexcept
    : View{parent}, _renderCallback{std::move(renderCallback)}
  {}

  explicit CanvasView(
    Window *window, std::function<void(View &, Renderer &)> renderCallback
  ) noexcept
    : View{window}, _renderCallback{std::move(renderCallback)}
  {}

  explicit CanvasView(
    View *parent, std::function<void(Renderer &)> renderCallback
  ) noexcept
    : View{parent},
      _renderCallback{[rc = std::move(renderCallback)](View &, Renderer &r) {
        rc(r);
      }}
  {}

  explicit CanvasView(
    Window *window, std::function<void(Renderer &)> renderCallback
  ) noexcept
    : View{window},
      _renderCallback{[rc = std::move(renderCallback)](View &, Renderer &r) {
        rc(r);
      }}
  {}

  void render(Renderer &r) override {
    _renderCallback(*this, r);
  }
};

} // namespace a8::gfx

#endif // A8_GFX_VIEW_H_

