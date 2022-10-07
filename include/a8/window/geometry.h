#ifndef A8_WINDOW_GEOMETRY_H_
#define A8_WINDOW_GEOMETRY_H_

#include <cstdint>
#include <type_traits>
#include <iosfwd>

namespace a8::win {

// Anchor {{{

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
  return static_cast<Anchor>(~static_cast<int>(a) & 0xF);
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

// }}}

// Point {{{

template<typename Coord> struct PointT;
template<typename Coord>
std::ostream &operator<<(std::ostream &, const PointT<Coord> &);

template<typename Coord>
struct PointT {
  using coord_type = Coord;

  Coord x;
  Coord y;

  PointT() = default;
  PointT(const PointT<Coord> &) = default;

  constexpr PointT(Coord x, Coord y) noexcept
    : x{x}, y{y}
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

  friend std::ostream &operator<< <Coord>(std::ostream &os,
                                          const PointT<Coord> &self);
};

template<bool scaled>
using PointS = PointT<std::conditional_t<scaled, float, int32_t>>;
using RawPoint = PointS<false>;
using Point = PointS<true>;

// }}}

// Size {{{

template<typename Coord> struct SizeT;
template<typename Coord>
std::ostream &operator<<(std::ostream &, const SizeT<Coord> &);

template<typename Coord>
struct SizeT {
  using coord_type = Coord;

  Coord width;
  Coord height;

  SizeT() = default;
  SizeT(const SizeT<Coord> &) = default;
  constexpr SizeT(Coord width, Coord height) noexcept
    : width{width}, height{height}
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

  friend std::ostream &operator<< <Coord>(std::ostream &os,
                                          const SizeT<Coord> &self);
};

template<bool scaled>
using SizeS = SizeT<std::conditional_t<scaled, float, int32_t>>;
using RawSize = SizeS<false>;
using Size = SizeS<true>;

// }}}

// Rect {{{

template<typename Coord> struct RectT;
template<typename Coord>
std::ostream &operator<<(std::ostream &, const RectT<Coord> &);

template<typename Coord>
struct RectT {
  using coord_type = Coord;

  Coord left;
  Coord top;
  Coord right;
  Coord bottom;

  RectT() = default;
  RectT(const RectT<Coord> &) = default;

  constexpr RectT(Coord left, Coord top, Coord right, Coord bottom) noexcept
    : left{left}, top{top}, right{right}, bottom{bottom}
  {}

  constexpr RectT(PointT<Coord> leftTop, SizeT<Coord> size) noexcept
    : left{leftTop.x}, top{leftTop.y},
      right{leftTop.x + size.width},
      bottom{leftTop.y + size.height}
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

  constexpr PointT<Coord> topLeft() const {
    return { left, top };
  }

  constexpr PointT<Coord> topRight() const {
    return { right, top };
  }

  constexpr PointT<Coord> bottomLeft() const {
    return { left, bottom };
  }

  constexpr PointT<Coord> bottomRight() const {
    return { right, bottom };
  }

  constexpr PointT<Coord> center() const {
    return { (left + right) / 2, (top + bottom) / 2 };
  }

  constexpr SizeT<Coord> size() const {
    return { right - left, bottom - top };
  }

  friend std::ostream &operator<< <Coord>(std::ostream &os,
                                          const RectT<Coord> &self);
};

template<bool scaled>
using RectS = RectT<std::conditional_t<scaled, float, int32_t>>;
using RawRect = RectS<false>;
using Rect = RectS<true>;

// }}}

} // namespace a8::win

#endif /* A8_WINDOW_GEOMETRY_H_ */
