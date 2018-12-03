#ifndef _a8_gfx_common_h_
#define _a8_gfx_common_h_

#include <cstdint>
#include <type_traits>
#include <iosfwd>

namespace a8::gfx {

template<typename XY>
struct xypair_t : public XY {
  using typename XY::coord;

  constexpr xypair_t() noexcept
    : XY()
  {}

  constexpr xypair_t(coord x, coord y) noexcept
    : XY(x, y)
  {}

  xypair_t(const xypair_t<XY> &other) = default;

  template<typename ZW, typename = typename XY::convert_from<ZW>>
  constexpr explicit xypair_t(const ZW &other) noexcept
    : XY(coord(other.xy_x()), coord(other.xy_y()))
  {}

  xypair_t<XY> &operator=(const xypair_t<XY> &other) = default;

  constexpr bool operator==(const xypair_t<XY> &other) const noexcept {
    return xy_x() == other.xy_x() &&
           xy_y() == other.xy_y();
  }
  constexpr bool operator!=(const xypair_t<XY> &other) const noexcept {
    return !(*this == other);
  }

  constexpr xypair_t<XY> operator-() {
    return {-xy_x(), -xy_y()};
  }

  constexpr xypair_t<XY> &operator+=(const xypair_t<XY> &other) {
    xy_x() += other.xy_x();
    xy_y() += other.xy_y();
    return *this;
  }

  constexpr friend xypair_t<XY> operator+(
    xypair_t<XY> self, const xypair_t<XY> &other
  )
  {
    return self += other;
  }

  constexpr xypair_t<XY> &operator-=(const xypair_t<XY> &other) {
    xy_x() -= other.xy_x();
    xy_y() -= other.xy_y();
    return *this;
  }

  constexpr friend xypair_t<XY> operator-(
    xypair_t<XY> self, const xypair_t<XY> &other
  )
  {
    return self -= other;
  }

  constexpr xypair_t<XY> &operator*=(const xypair_t<XY> &amounts) {
    xy_x() *= amounts.xy_x();
    xy_y() *= amounts.xy_y();
    return *this;
  }

  constexpr friend xypair_t<XY> operator*(
    xypair_t<XY> self, const xypair_t<XY> &amounts
  )
  {
    return self *= amounts;
  }

  constexpr xypair_t<XY> &operator*=(coord amount) {
    xy_x() *= amount;
    xy_y() *= amount;
    return *this;
  }

  constexpr friend xypair_t<XY> operator*(
    xypair_t<XY> self, coord amount
  )
  {
    return self *= amount;
  }

  constexpr friend xypair_t<XY> operator*(
    coord amount, xypair_t<XY> self
  )
  {
    return self *= amount;
  }

  constexpr xypair_t<XY> &operator/=(const xypair_t<XY> &amounts) {
    xy_x() /= amounts.xy_x();
    xy_y() /= amounts.xy_y();
    return *this;
  }

  constexpr friend xypair_t<XY> operator/(
    xypair_t<XY> self, const xypair_t<XY> &amounts
  )
  {
    return self /= amounts;
  }

  constexpr xypair_t<XY> &operator/=(coord amount) {
    xy_x() /= amount;
    xy_y() /= amount;
    return *this;
  }

  constexpr friend xypair_t<XY> operator/(
    xypair_t<XY> self, coord amount
  )
  {
    return self /= amount;
  }

  template<typename ZW, typename = typename ZW::convert_from<XY>>
  constexpr explicit operator ZW() const {
    return ZW{ typename ZW::coord(xy_x()), typename ZW::coord(xy_y()) };
  }
};

template<typename Coord>
struct basic_point_t {
  using coord = Coord;

  template<typename Coord2>
  using convert_from =
    std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>;

  constexpr basic_point_t() noexcept
    : x{}, y{}
  {}

  constexpr basic_point_t(coord x, coord y) noexcept
    : x{x}, y{y}
  {}

  friend std::ostream &operator<<(std::ostream &os, const basic_point_t<Coord> &self);

  coord x;
  coord y;

  coord &xy_x() { return x; }
  const coord &xy_x() const { return x; }
  coord &xy_y() { return y; }
  const coord &xy_y() const { return y; }
};

template<typename Coord>
struct basic_size_t {
  using coord = Coord;

  template<typename Coord2>
  using convert_from =
    std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>;

  constexpr basic_size_t() noexcept
    : width{}, height{}
  {}

  constexpr basic_size_t(coord w, coord h) noexcept
    : width{w}, height{h}
  {}

  friend std::ostream &operator<<(std::ostream &os, const basic_size_t<Coord> &self);

  coord width;
  coord height;

  coord &xy_x() { return width; }
  const coord &xy_x() const { return width; }
  coord &xy_y() { return height; }
  const coord &xy_y() const { return height; }
};

template<typename Coord>
struct PointT {
  Coord x;
  Coord y;

  PointT() = default;
  PointT(const PointT<Coord> &) = default;

  constexpr PointT(Coord x, Coord y) noexcept
    : x{x}, y{y}
  {}

  template<typename = std::enable_if_t<!std::is_reference_v<Coord>>>
  constexpr explicit PointT(const PointT<Coord &> &ref) noexcept
    : PointT{ref.x, ref.y}
  {}

  template<
    typename Coord2,
    typename = std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>
  >
  constexpr explicit PointT(const PointT<Coord2> &other) noexcept
    : PointT{static_cast<Coord>(other.x), static_cast<Coord>(other.y)}
  {}

  PointT<Coord> &operator=(const PointT<Coord> &) = default;

  constexpr bool operator==(const PointT<Coord> &other) const {
    return x == other.x && y == other.y;
  }
  constexpr bool operator!=(const PointT<Coord> &other) const {
    return !(*this == other);
  }

  friend constexpr PointT<Coord> operator-(PointT<Coord> self) {
    return {-self.x, -self.y};
  }

  constexpr PointT<Coord> &operator+=(const PointT<Coord> &other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  friend constexpr PointT<Coord> operator+(PointT<Coord> self, const PointT<Coord> &other) {
    self += other;
    return self;
  }

  constexpr PointT<Coord> &operator-=(const PointT<Coord> &other) {
    return *this = -other;
  }
  friend constexpr PointT<Coord> operator-(PointT<Coord> self, const PointT<Coord> &other) {
    self -= other;
    return self;
  }

  constexpr PointT<Coord> &operator*=(Coord scale) {
    x *= scale;
    y *= scale;
    return *this;
  }
  friend constexpr PointT<Coord> operator*(PointT<Coord> self, Coord scale) {
    self *= scale;
    return self;
  }

  constexpr PointT<Coord> &operator*=(const PointT<Coord> &scale) {
    x *= scale.x;
    y *= scale.y;
    return *this;
  }
  friend constexpr PointT<Coord> operator*(PointT<Coord> self, const PointT<Coord> &scale) {
    self *= scale;
    return self;
  }

  constexpr PointT<Coord> &operator/=(Coord scale) {
    x /= scale;
    y /= scale;
    return *this;
  }
  friend constexpr PointT<Coord> operator/(PointT<Coord> self, Coord scale) {
    self /= scale;
    return self;
  }

  constexpr PointT<Coord> &operator/=(const PointT<Coord> &scale) {
    x /= scale.x;
    y /= scale.y;
    return *this;
  }
  friend constexpr PointT<Coord> operator/(PointT<Coord> self, const PointT<Coord> &scale) {
    self /= scale;
    return self;
  }
};

template<bool scaled>
using PointS = PointT<std::conditional_t<scaled, float, int32_t>>;
using RawPoint = PointS<false>;
using Point = PointS<true>;

template<typename Coord>
struct SizeT {
  Coord width;
  Coord height;

  SizeT() = default;
  SizeT(const SizeT<Coord> &) = default;
  constexpr SizeT(Coord width, Coord height) noexcept
    : width{width}, height{height}
  {}

  template<typename = std::enable_if_t<!std::is_reference_v<Coord>>>
  constexpr SizeT(const SizeT<Coord &> &ref) noexcept
    : SizeT{ref.width, ref.height}
  {}

  template<
    typename Coord2,
    typename = std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>
  >
  constexpr explicit SizeT(const SizeT<Coord2> &other) noexcept
    : SizeT{static_cast<Coord>(other.width), static_cast<Coord>(other.height)}
  {}

  SizeT<Coord> &operator=(const SizeT<Coord> &) = default;

  constexpr bool operator==(const SizeT<Coord> &other) const {
    return width == other.width && height == other.height;
  }
  constexpr bool operator!=(const SizeT<Coord> &other) const {
    return !(*this == other);
  }

  constexpr SizeT<Coord> &operator+=(const SizeT<Coord> &other) {
    width += other.width;
    height += other.height;
    return *this;
  }
  friend constexpr SizeT<Coord> operator+(SizeT<Coord> self, const SizeT<Coord> &other) {
    self += other;
    return self;
  }

  constexpr SizeT<Coord> &operator-=(const SizeT<Coord> &other) {
    width -= other.width;
    height -= other.height;
    return *this;
  }
  friend constexpr SizeT<Coord> operator-(SizeT<Coord> self, const SizeT<Coord> &other) {
    self -= other;
    return self;
  }

  constexpr SizeT<Coord> &operator*=(Coord scale) {
    width *= scale;
    height *= scale;
    return *this;
  }
  friend constexpr SizeT<Coord> operator*(SizeT<Coord> self, Coord scale) {
    self *= scale;
    return self;
  }

  constexpr SizeT<Coord> &operator*=(const SizeT<Coord> &scale) {
    width *= scale.width;
    height *= scale.height;
    return *this;
  }
  friend constexpr SizeT<Coord> operator*(SizeT<Coord> self, const SizeT<Coord> &scale) {
    self *= scale;
    return self;
  }

  constexpr SizeT<Coord> &operator/=(Coord scale) {
    width /= scale;
    height /= scale;
    return *this;
  }
  friend constexpr SizeT<Coord> operator/(SizeT<Coord> self, Coord scale) {
    self /= scale;
    return self;
  }

  constexpr SizeT<Coord> &operator/=(const SizeT<Coord> &scale) {
    width /= scale.width;
    height /= scale.height;
    return *this;
  }
  friend constexpr SizeT<Coord> operator/(SizeT<Coord> self, const SizeT<Coord> &scale) {
    self /= scale;
    return self;
  }
};

template<bool scaled>
using SizeS = SizeT<std::conditional_t<scaled, float, uint32_t>>;
using RawSize = SizeS<false>;
using Size = SizeS<true>;

template<typename Coord>
struct RectT {
  Coord left;
  Coord top;
  Coord right;
  Coord bottom;

  RectT() = default;
  RectT(const RectT<Coord> &) = default;

  constexpr RectT(Coord left, Coord top, Coord right, Coord bottom) noexcept
    : left{left}, top{top}, right{right}, bottom{bottom}
  {}

  template<typename = std::enable_if_t<!std::is_reference_v<Coord>>>
  constexpr RectT(const RectT<Coord &> &ref) noexcept
    : RectT{ ref.left, ref.top, ref.right, ref.bottom }
  {}

  template<
    typename Coord2,
    typename = std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>
  >
  constexpr explicit RectT(const RectT<Coord2> &other) noexcept
    : RectT{
        static_cast<Coord>(left),
        static_cast<Coord>(top),
        static_cast<Coord>(right),
        static_cast<Coord>(bottom)
      }
  {}

  RectT<Coord> &operator=(const RectT<Coord> &) = default;

  constexpr bool operator==(const RectT<Coord> &other) const {
    return left == other.left &&
           top == other.top &&
           right == other.right &&
           bottom == other.bottom;
  }
  constexpr bool operator!=(const RectT<Coord> &other) const {
    return !(*this == other);
  }

  // Flip left/right, top/bottom.
  constexpr friend RectT<Coord> operator-(RectT<Coord> self) {
    return { self.right, self.bottom, self.left, self.top };
  }

  constexpr PointT<Coord &> topLeft() const {
    return { left, top };
  }

  constexpr PointT<Coord &> bottomRight() const {
    return { right, bottom };
  }

  constexpr SizeT<Coord> size() const {
    return { right - left, bottom - top };
  }
};

template<bool scaled>
using RectS = RectT<std::conditional_t<scaled, float, int32_t>>;
using RawRect = RectS<false>;
using Rect = RectS<true>;


enum class Anchor {
  None          = 0,
  Left          = 0x1,
  Right         = 0x2,
  /// Both left and right
  Horizontal    = 0x3,
  Top           = 0x4,
  TopLeft       = 0x5,
  TopRight      = 0x6,
  /// Top, left, and right
  FillTop       = 0x7,
  Bottom        = 0x8,
  BottomLeft    = 0x9,
  BottomRight   = 0xA,
  /// Bottom, left, and right
  FillBottom    = 0xB,
  /// Top and bottom
  Vertical      = 0xC,
  /// Top, bottom, and left
  FillLeft      = 0xD,
  /// Top, bottom, and right
  FillRight     = 0xE,
  /// All edges
  Expand        = 0xF,
};
/// Union: Points in either a1 or a2.
constexpr Anchor operator|(Anchor a1, Anchor a2) {
  return static_cast<Anchor>(static_cast<int>(a1) | static_cast<int>(a2));
}
/// Union: Same as operator|.
constexpr Anchor operator+(Anchor a1, Anchor a2) {
  return a1 | a2;
}
/// Intersection: Points occurring in both a1 and a2.
constexpr Anchor operator&(Anchor a1, Anchor a2) {
  return static_cast<Anchor>(static_cast<int>(a1) & static_cast<int>(a2));
}
/// Set difference: Removes points in a2 from the set a1.
constexpr Anchor operator-(Anchor a1, Anchor a2) {
  return static_cast<Anchor>(static_cast<int>(a1) & ~static_cast<int>(a2));
}
/// Exclusion: Points in either set but not both.
constexpr Anchor operator^(Anchor a1, Anchor a2) {
  return static_cast<Anchor>((static_cast<int>(a1) ^ static_cast<int>(a2)) & 0xF);
}
/// Inverse: Changes all points to the opposite state.
constexpr Anchor operator~(Anchor a) {
  return static_cast<Anchor>(~static_cast<int>(a));
}
/// Reflection: Flips left/right, top/bottom, but when both are set, both remain.
constexpr Anchor operator-(Anchor a) {
  const int i = static_cast<int>(a);
  return static_cast<Anchor>(((i & 0x5) << 1) | ((i & 0xA) >> 1));
}
constexpr bool operator!(Anchor a) {
  return a == Anchor::None;
}
std::ostream &operator<<(std::ostream &os, Anchor a);

template<typename Q>
struct ptpair_t : public Q {
private:
  constexpr ptpair_t<Q> adjust(
    const basic_point_t<typename Q::coord> &point,
    const basic_size_t<typename Q::coord> &size,
    Anchor anchor
  ) noexcept
  {
    typename Q::coord x[2];
    typename Q::coord y[2];
    if (anchor & Anchor::Left) {
      x[0] = point.x;
      x[1] = point.x + size.width;
    } else if (anchor & Anchor::Right) {
      x[0] = point.x - size.width;
      x[1] = point.x;
    } else {
      x[0] = point.x - size.width / 2;
      x[1] = x[0] + size.width;
    }

    if (anchor & Anchor::Top) {
      y[0] = point.y;
      y[1] = point.y + size.height;
    } else if (anchor & Anchor::Bottom) {
      y[0] = point.y - size.height;
      y[1] = point.y;
    } else {
      y[0] = point.y - size.height / 2;
      y[1] = y[0] + size.height;
    }

    return {x[0], y[0], x[1], y[1]};
  }

public:
  using typename Q::coord;

  constexpr ptpair_t() noexcept
    : Q{}
  {}

  constexpr ptpair_t(coord x1, coord y1, coord x2, coord y2) noexcept
    : Q{x1, y1, x2, y2}
  {}

  constexpr explicit ptpair_t(
    const basic_point_t<coord> &point,
    const basic_size_t<coord> &size,
    Anchor anchor = Anchor::TopLeft
  ) noexcept
    : ptpair_t{adjust(point, size, anchor)}
  {}

  constexpr explicit ptpair_t(
    const basic_point_t<coord> &pt1,
    const basic_point_t<coord> &pt2
  ) noexcept
    : Q{pt1.x, pt1.y, pt2.x, pt2.y}
  {}

  constexpr ptpair_t(const ptpair_t<Q> &other) noexcept
    : Q{other.c1x(), other.c1y(), other.c2x(), other.c2y()}
  {}

  constexpr ptpair_t<Q> &operator=(const ptpair_t<Q> &other) noexcept {
    c1x() = other.c1x();
    c1y() = other.c1y();
    c2x() = other.c2x();
    c2y() = other.c2y();
  }

  constexpr bool operator==(const ptpair_t<Q> &other) noexcept {
    return c1x() == other.c1x() && c1y() == other.c1y() &&
      c2x() == other.c2x() && c2y() == other.c2y();
  }

  constexpr bool operator!=(const ptpair_t<Q> &other) noexcept {
    return !(*this == other);
  }

  constexpr ptpair_t<Q> operator-() noexcept {
    return {c2x(), c2y(), c1x(), c2y()};
  }

  constexpr ptpair_t<Q> &operator+=(const basic_point_t<Q> &point) noexcept {
    c1x() += point.x;
    c1y() += point.y;
    c2x() += point.x;
    c2y() += point.y;
    return *this;
  }

  constexpr ptpair_t<Q> &operator-=(const basic_point_t<Q> &point) noexcept {
    return *this += -point;
  }
};

template<typename Coord>
struct basic_rect_t {
  using coord = Coord;

  template<typename Coord2>
  using convert_to =
    std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>, Coord2>;

  coord left;
  coord top;
  coord right;
  coord bottom;

  constexpr basic_rect_t() noexcept
    : left{}, top{}, right{}, bottom{}
  {}

  constexpr basic_rect_t(coord left, coord top, coord right, coord bottom) noexcept
    : left{left}, top{top}, right{right}, bottom{bottom}
  {}

  friend std::ostream &operator<<(std::ostream &os, const basic_rect_t<Coord> &self);

  coord &c1x() { return left; }
  const coord &c1x() const { return left; }
  coord &c1y() { return top; }
  const coord &c1y() const { return top; }
  coord &c2x() { return right; }
  const coord &c2x() const { return right; }
  coord &c2y() { return bottom; }
  const coord &c2y() const { return bottom; }
};

template<typename Coord>
struct basic_line_t {
  using coord = Coord;

  template<typename Coord2>
  using convert_from =
    std::enable_if_t<std::is_nothrow_constructible_v<Coord2, Coord>>;

  coord x1;
  coord y1;
  coord x2;
  coord y2;

  constexpr basic_line_t() noexcept
    : x1{}, y1{}, x2{}, y2{}
  {}

  constexpr basic_line_t(coord x1, coord y1, coord x2, coord y2) noexcept
    : x1{x1}, y1{y1}, x2{x2}, y2{y2}
  {}

  friend std::ostream &operator<<(std::ostream &os, const basic_line_t<Coord> &self);

  coord &c1x() { return x1; }
  const coord &c1x() const { return x1; }
  coord &c1y() { return y1; }
  const coord &c1y() const { return y1; }
  coord &c2x() { return x2; }
  const coord &c2x() const { return x2; }
  coord &c2y() { return y2; }
  const coord &c2y() const { return y2; }
};

extern template struct ptpair_t<basic_line_t<int32_t>>;
extern template struct ptpair_t<basic_line_t<float>>;
template<typename Coord>
using LineT = ptpair_t<basic_line_t<Coord>>;
template<bool scaled>
using LineS = LineT<std::conditional_t<scaled, float, int32_t>>;
using RawLine = LineS<false>;
using Line = LineS<true>;

} // namespace a8::gfx

#endif // _a8_gfx_common_h_
