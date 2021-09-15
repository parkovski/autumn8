#ifndef A8_GFX_VIEWS_VIEW_H_
#define A8_GFX_VIEWS_VIEW_H_

#include "../events.h"

#include <boost/signals2.hpp>

namespace a8::gfx {

class Window;

class View {
public:
  template<class E>
  using Signal = ::a8::gfx::Signal<View, E>;

  template<class E>
  using Slot = ::a8::gfx::Slot<View, E>;

  View() = default;

  View(const View &) = delete;
  View &operator=(const View &) = delete;

  View(View &&) = delete;
  View &operator=(View &&) = delete;

  virtual ~View() = 0;

  virtual void init(Window *window) = 0;
  virtual void resize(RawPoint pos, RawSize size) = 0;
};

} // namespace a8::gfx

#endif // A8_GFX_VIEWS_VIEW_H_
