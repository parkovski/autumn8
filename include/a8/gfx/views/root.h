#ifndef _a8_gfx_views_root_h_
#define _a8_gfx_views_root_h_

#include "a8/gfx/view.h"

namespace a8::gfx {

class ViewRoot : public View {
  void render(Renderer &r, const Rect &rect) override;
};

} // namespace a8::gfx

#endif // _a8_gfx_views_root_h_
