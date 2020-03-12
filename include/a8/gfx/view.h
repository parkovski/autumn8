#ifndef A8_GFX_VIEW_H_
#define A8_GFX_VIEW_H_

#include <boost/signals2.hpp>

#include <type_traits>
#include <vector>

namespace a8::gfx {

class Renderer;

class View {
public:
  template<typename E>
  using sig_t = boost::signals2::signal<bool (View &, const E &)>;

  explicit View() noexcept
    : _parent{nullptr}
  {}

  View(const View &) = delete;
  View &operator=(const View &) = delete;

  View(View &&) = delete;
  View &operator=(View &&) = delete;

  virtual ~View() = 0;

  View *parent() {
    return _parent;
  }

  virtual void render(Renderer &r) = 0;

private:
  View *_parent;
};

template<typename F>
class CanvasView : public View {
public:
  template<typename = std::enable_if_t<
    std::is_invocable_v<F, View &, Renderer &>
  >>
  explicit CanvasView(F &&renderCallback)
    noexcept(std::is_nothrow_move_constructible_v<F>)
    : _renderCallback{std::move(renderCallback)}
  {}

  ~CanvasView() = default;

  void render(Renderer &r) override {
    _renderCallback(*this, r);
  }

private:
  F _renderCallback;
};

} // namespace a8::gfx

#endif // A8_GFX_VIEW_H_
