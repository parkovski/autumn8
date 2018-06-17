#ifndef _a8_gfx_view_h_
#define _a8_gfx_view_h_

#include "a8/gfx/common.h"

#include "nanovg.h"
#include "yoga/Yoga.h"

namespace a8::gfx {

class View {
public:
  virtual void render(NVGcontext *vg, const Rect &rect) = 0;
};

class FlexView : public View {
public:
  virtual void render(NVGcontext *vg, const Rect &rect) override;
};

class TextView : public View {
  std::string _text;

public:
  virtual void render(NVGcontext *vg, const Rect &rect) override;
};

}

#endif // _a8_gfx_view_h_
