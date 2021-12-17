#ifndef A8_UTIL_CSTRING_VIEW_H_
#define A8_UTIL_CSTRING_VIEW_H_

#include <string>
#include <string_view>

namespace a8 {

template<class CharT, class Traits = std::char_traits<CharT>>
class basic_cstring_view {
public:
  using traits_type = Traits;
  using value_type = CharT;
  using pointer = CharT *;
  using const_pointer = const CharT *;
  using reference = CharT &;
  using const_reference = const CharT &;
  struct const_iterator;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  constexpr basic_cstring_view() noexcept
    : _data{nullptr}, _length{0}
  {}

  constexpr basic_cstring_view(const basic_cstring_view &) noexcept = default;

  constexpr basic_cstring_view(const CharT *s)
    : _data{s}, _length{0}
  {
    if (s) {
      _length = traits_type::length(s);
    }
  }

private:
  constexpr basic_cstring_view(const CharT *s, size_type size) noexcept
    : _data{s}, _length{size}
  {}

public:
  constexpr static basic_cstring_view
  __construct_private(const CharT *s, size_type size) noexcept {
    return {s, size};
  }

  constexpr basic_cstring_view(const std::basic_string<CharT, Traits> &s)
    : _data{s.c_str()}, _length{s.size()}
  {}

  constexpr basic_cstring_view(std::nullptr_t) = delete;

  constexpr basic_cstring_view &operator=(const basic_cstring_view &)
                                          noexcept = default;

  struct const_iterator {};

  constexpr const_iterator begin() const noexcept {
    return {};
  }

  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }

  constexpr const_iterator end() const noexcept {
    return {};
  }

  constexpr const_iterator cend() const noexcept {
    return end();
  }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return {};
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  constexpr const_reverse_iterator rend() const noexcept {
    return {};
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return rend();
  }

  constexpr const_reference operator[](size_type pos) const {
    return _data[pos];
  }

  constexpr const_reference at(size_type pos) const {
    return _data[pos];
  }

  constexpr const_reference front() const {
    return _data[0];
  }

  constexpr const_reference back() const {
    if (_length == 0) {
      return _data[0];
    }
    return _data[_length - 1];
  }

  constexpr const_pointer data() const noexcept {
    return _data;
  }

  constexpr const_pointer c_str() const noexcept {
    return _data;
  }

  constexpr size_type size() const noexcept {
    return _length;
  }

  constexpr size_type length() const noexcept {
    return size();
  }

  constexpr size_type max_size() const noexcept {
    return std::basic_string_view<CharT, Traits>{}.max_size() - 1;
  }

  // [[nodiscard]]
  constexpr bool empty() const noexcept {
    return size() == 0;
  }

  constexpr void remove_prefix(size_type n) {
    _data += n;
    _length -= n;
  }

  constexpr void swap(basic_cstring_view &v) noexcept {
    basic_cstring_view temp{*this};
    *this = v;
    v = temp;
  }

  //constexpr
  size_type copy(CharT *dest, size_type count, size_type pos = 0) const {
    // TODO
    return 0;
  }

  size_type c_copy(CharT *dest, size_type count, size_type pos = 0) const {
    // TODO
    return 0;
  }

  constexpr basic_cstring_view substr(size_type pos = 0) {
    if (pos >= size()) {
      return {};
    }
    return {_data + pos, _length - pos};
  }

  //compare...

  constexpr operator std::basic_string_view<CharT, Traits>() const noexcept {
    return {_data, _length};
  }

  constexpr operator std::basic_string<CharT, Traits>() const {
    return {_data, _length};
  }

private:
  const CharT *_data;
  size_t _length;
};

template<class CharT, class Traits>
constexpr bool operator==(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
      == std::basic_string_view<CharT, Traits>{b};
}

#if __cplusplus >= 201907L

template<class CharT, class Traits>
constexpr auto operator<=>(basic_cstring_view<CharT, Traits> a,
                           basic_cstring_view<CharT, Traits> b) noexcept {
return std::basic_string_view<CharT, Traits>{a}
   <=> std::basic_string_view<CharT, Traits>{b};
}

#else // __cplusplus >= 201907L

template<class CharT, class Traits>
constexpr bool operator!=(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
      != std::basic_string_view<CharT, Traits>{b};
}

template<class CharT, class Traits>
constexpr bool operator<(basic_cstring_view<CharT, Traits> a,
                         basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
       < std::basic_string_view<CharT, Traits>{b};
}

template<class CharT, class Traits>
constexpr bool operator<=(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
      <= std::basic_string_view<CharT, Traits>{b};
}

template<class CharT, class Traits>
constexpr bool operator>(basic_cstring_view<CharT, Traits> a,
                         basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
       > std::basic_string_view<CharT, Traits>{b};
}

template<class CharT, class Traits>
constexpr bool operator>=(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return std::basic_string_view<CharT, Traits>{a}
      >= std::basic_string_view<CharT, Traits>{b};
}

#endif // __cplusplus > 201907L

using cstring_view = basic_cstring_view<char>;
#if __cplusplus >= 201811L
  using u8cstring_view = basic_cstring_view<char8_t>;
#else
  using u8cstring_view = basic_cstring_view<char>;
#endif
using u16cstring_view = basic_cstring_view<char16_t>;
using u32cstring_view = basic_cstring_view<char32_t>;
using wcstring_view = basic_cstring_view<wchar_t>;

namespace literals {

constexpr cstring_view operator "" _csv(const char *str, size_t len) noexcept {
  return cstring_view::__construct_private(str, len);
}

#if __cplusplus >= 201811L
constexpr u8cstring_view operator "" _csv(const char8_t *str, size_t len)
                                          noexcept {
  return u8cstring_view::__construct_private(str, len);
}
#endif

constexpr u16cstring_view operator "" _csv(const char16_t *str, size_t len)
                                           noexcept {
  return u16cstring_view::__construct_private(str, len);
}

constexpr u32cstring_view operator "" _csv(const char32_t *str, size_t len)
                                           noexcept {
  return u32cstring_view::__construct_private(str, len);
}

constexpr wcstring_view operator "" _csv(const wchar_t *str, size_t len)
                                         noexcept {
  return wcstring_view::__construct_private(str, len);
}

} // namespace literals

template<class CharT, class Traits>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os,
           basic_cstring_view<CharT, Traits> &v) {
  return os << std::basic_string_view<CharT, Traits>{v};
}

} // namespace a8

namespace std {

template<> struct hash<a8::cstring_view> {
  size_t operator()(const a8::cstring_view &v) const {
    return hash<string_view>{}(v);
  }
};

// template<> struct hash<a8::u8cstring_view> {
//   size_t operator(const a8::u8cstring_view &v) const {
//     return hash<u8string_view>{}(v);
//   }
// };

template<> struct hash<a8::u16cstring_view> {
  size_t operator()(const a8::u16cstring_view &v) const {
    return hash<u16string_view>{}(v);
  }
};

template<> struct hash<a8::u32cstring_view> {
  size_t operator()(const a8::u32cstring_view &v) const {
    return hash<u32string_view>{}(v);
  }
};

template<> struct hash<a8::wcstring_view> {
  size_t operator()(const a8::wcstring_view &v) const {
    return hash<wstring_view>{}(v);
  }
};

} // namespace std

#endif // A8_UTIL_CSTRING_VIEW_H_
