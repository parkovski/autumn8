#include "a8/window/geometry.h"
#include "a8/window/events.h"

#include <ostream>

namespace a8::win {

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

std::ostream &operator<<(std::ostream &os, Anchor a) {
  switch (a) {
  case Anchor::Left: os << "Left"; break;
  case Anchor::Right: os << "Right"; break;
  case Anchor::Horizontal: os << "Horizontal"; break;
  case Anchor::Top: os << "Top"; break;
  case Anchor::TopLeft: os << "TopLeft"; break;
  case Anchor::TopRight: os << "TopRight"; break;
  case Anchor::FillTop: os << "FillTop"; break;
  case Anchor::Bottom: os << "Bottom"; break;
  case Anchor::BottomLeft: os << "BottomLeft"; break;
  case Anchor::BottomRight: os << "BottomRight"; break;
  case Anchor::FillBottom: os << "FillBottom"; break;
  case Anchor::Vertical: os << "Vertical"; break;
  case Anchor::FillLeft: os << "FillLeft"; break;
  case Anchor::FillRight: os << "FillRight"; break;
  case Anchor::Expand: os << "Expand"; break;
  default: os << "None"; break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, ViewEvent e) {
#define CASE(evt) case ViewEvent::evt: os << #evt; break
  switch (e) {
    CASE(Initialize);
    CASE(Destroy);
    default: break;
  }
#undef CASE
  return os;
}

std::ostream &operator<<(std::ostream &os, WindowEvent e) {
#define CASE(evt) case WindowEvent::evt: os << #evt; break
  switch (e) {
    CASE(Close);
    CASE(Minimize);
    CASE(Maximize);
    CASE(RestoreSize);
    CASE(FullScreen);
    CASE(Focus);
    CASE(Unfocus);
  }
#undef CASE
  return os;
}

std::ostream &operator<<(std::ostream &os, SystemEvent e) {
#define CASE(evt) case SystemEvent::evt: os << #evt; break
  switch (e) {
    CASE(Pause);
    CASE(Resume);
    CASE(Back);
    default: break;
  }
#undef CASE
  return os;
}

} // namespace a8::win
