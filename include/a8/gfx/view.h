#ifndef A8_GFX_VIEW_H_
#define A8_GFX_VIEW_H_

#include "a8/gfx/events.h"
#include "a8/gfx/renderer.h"

namespace a8::gfx {

class Window;
class RootView;

class View {
  View *_parent;
  RootView *_root;

  friend class RootView;

protected:
  EventSignals<ViewEvent> _signals;

public:
  View() = delete;

  explicit View(View *parent) noexcept
    : _parent{parent}, _root{parent ? parent->_root : nullptr}
  {}

  View(const View &) = delete;
  View &operator=(const View &) = delete;

  View(View &&) = default;
  View &operator=(View &&) = default;

  virtual ~View() = 0;

  View *parent() {
    return _parent;
  }

  RootView *root() {
    return _root;
  }

  Window *window();

  /// @type E The event type.
  /// @param slot The callback function.
  template<typename E>
  auto connect(boost::signals2::slot<bool (View &, const E &)> slot) {
    return _signals.get<E>().connect(slot);
  }

  virtual void render(Renderer &r) = 0;
};

class RootView : public View {
  Window *_window;

public:
  explicit RootView(Window *window) noexcept
    : View{nullptr}, _window{window}
  {
    _root = this;
  }

  Window *window() {
    return _window;
  }

  void render(Renderer &r) override;
};

inline Window *View::window() {
  return _root->window();
}

class CanvasView : public View {
  std::function<void(View &, Renderer &)> _renderCallback;

public:
  explicit CanvasView(
    View *parent, std::function<void(View &, Renderer &)> renderCallback
  ) noexcept
    : View{parent}, _renderCallback{renderCallback}
  {}

  void render(Renderer &r) override {
    _renderCallback(*this, r);
  }
};

} // namespace a8::gfx

#endif // A8_GFX_VIEW_H_

