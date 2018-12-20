#include "a8/gfx/common.h"

#include <ostream>

namespace a8::gfx {

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const PointT<Coord> &self) {
  return os << "(" << self.x << ", " << self.y << ")";
}

// Explicit instantiations
template std::ostream &
operator<< <RawPoint::coord_type>(std::ostream &, const RawPoint &);
template std::ostream &
operator<< <Point::coord_type>(std::ostream &, const Point &);

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const SizeT<Coord> &self) {
  return os << "(" << self.width << " x " << self.height << ")";
}

// Explicit instantiations
template std::ostream &
operator<< <RawSize::coord_type>(std::ostream &, const RawSize &);
template std::ostream &
operator<< <Size::coord_type>(std::ostream &, const Size &);

template<typename Coord>
std::ostream &
operator<<(std::ostream &os, const RectT<Coord> &self) {
  return os << "{(" << self.left << ", " << self.top <<
    "), (" << self.right << ", " << self.bottom << ")}";
}

// Explicit instantiations
template std::ostream &
operator<< <RawRect::coord_type>(std::ostream &, const RawRect &);
template std::ostream &
operator<< <Rect::coord_type>(std::ostream &, const Rect &);

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

}
