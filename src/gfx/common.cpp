#include "a8/gfx/common.h"

#include <ostream>

namespace a8::gfx {

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const basic_point_t<Coord> &self) {
  os << "Point(" << self.x << ", " << self.y << ")";
}

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const basic_size_t<Coord> &self) {
  return os << "Size(" << self.width << " x " << self.height << ")";
}

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const basic_rect_t<Coord> &self) {
  return os << "Rect{(" << self.left << ", " << self.top <<
    ") - (" << self.right << ", " << self.bottom << ")}";
}

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const basic_line_t<Coord> &self) {
  return os << "Line{(" << self.x1 << ", " << self.y1 <<
    ") - (" << self.x2 << ", " << self.y2 << ")}";
}


std::ostream &a8::gfx::operator<<(std::ostream &os, Anchor a) {
  switch (a) {
  case Anchor::Left: os << "Anchor::Left"; break;
  case Anchor::Right: os << "Anchor::Right"; break;
  case Anchor::Horizontal: os << "Anchor::Horizontal"; break;
  case Anchor::Top: os << "Anchor::Top"; break;
  case Anchor::TopLeft: os << "Anchor::TopLeft"; break;
  case Anchor::TopRight: os << "Anchor::TopRight"; break;
  case Anchor::FillTop: os << "Anchor::FillTop"; break;
  case Anchor::Bottom: os << "Anchor::Bottom"; break;
  case Anchor::BottomLeft: os << "Anchor::BottomLeft"; break;
  case Anchor::BottomRight: os << "Anchor::BottomRight"; break;
  case Anchor::FillBottom: os << "Anchor::FillBottom"; break;
  case Anchor::Vertical: os << "Anchor::Vertical"; break;
  case Anchor::FillLeft: os << "Anchor::FillLeft"; break;
  case Anchor::FillRight: os << "Anchor::FillRight"; break;
  case Anchor::Expand: os << "Anchor::Expand"; break;
  default: os << "Anchor::None"; break;
  }
  return os;
}

template struct xypair_t<basic_point_t<int32_t>>;
template struct xypair_t<basic_point_t<float>>;

template struct xypair_t<basic_size_t<int32_t>>;
template struct xypair_t<basic_size_t<float>>;

template struct ptpair_t<basic_rect_t<int32_t>>;
template struct ptpair_t<basic_rect_t<float>>;

template struct ptpair_t<basic_line_t<int32_t>>;
template struct ptpair_t<basic_line_t<float>>;
}
