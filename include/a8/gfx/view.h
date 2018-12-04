#ifndef A8_GFX_VIEW_H_
#define A8_GFX_VIEW_H_

#include "a8/gfx/common.h"
#include "a8/gfx/renderer.h"

#include <string>
//#include "yoga/Yoga.h"

namespace a8::gfx {

class View {
public:
  virtual void render(Renderer &r, const Rect &rect) = 0;
};

class FlexView : public View {
public:
  virtual void render(Renderer &r, const Rect &rect) override;
};

class TextView : public View {
  std::string _text;

public:
  virtual void render(Renderer &r, const Rect &rect) override;
};

}

#endif // A8_GFX_VIEW_H_

