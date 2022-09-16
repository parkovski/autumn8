#ifndef A8_UTIL_CSTRING_VIEW_H_
#define A8_UTIL_CSTRING_VIEW_H_

#include <string>
#include <string_view>

namespace a8 {

// A mostly `std::string_view` compatible class for APIs taking a null
// terminated C string. Places where compatiblility with string_view are
// not present are due to the requirement that the string terminates with
// a null character and the string cannot be modified by this view.
// This class never contains a null data pointer.
template<class CharT, class Traits = std::char_traits<CharT>>
class basic_cstring_view {
  static constexpr const CharT empty_string[1] = {0};

public:
  using traits_type = Traits;
  using value_type = CharT;
  using pointer = CharT *;
  using const_pointer = const CharT *;
  using reference = CharT &;
  using const_reference = const CharT &;
  using const_iterator = const_pointer;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  constexpr basic_cstring_view() noexcept
    : _data{empty_string}, _length{0}
  {}

  constexpr basic_cstring_view(const basic_cstring_view &) noexcept = default;

  constexpr basic_cstring_view(const CharT *s)
    : _data{s}, _length{npos}
  {}

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

  constexpr const_iterator cbegin() const noexcept {
    return _data;
  }

  constexpr const_iterator cend() const noexcept {
    return _data + size();
  }

  constexpr const_iterator begin() const noexcept {
    return cbegin();
  }

  constexpr const_iterator end() const noexcept {
    return cend();
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return cend();
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return cbegin();
  }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return crbegin();
  }

  constexpr const_reverse_iterator rend() const noexcept {
    return crend();
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
    if (empty()) {
      return _data[0];
    }
    return _data[size() - 1];
  }

  constexpr const_pointer data() const noexcept {
    return _data;
  }

  constexpr const_pointer c_str() const noexcept {
    return _data;
  }

  constexpr size_type size() const noexcept {
    if (_length == npos) {
      auto length = Traits::length(_data);
      _length = length;
      return length;
    } else {
      return _length;
    }
  }

  constexpr size_type length() const noexcept {
    return size();
  }

  constexpr size_type max_size() const noexcept {
    return std::basic_string_view<CharT, Traits>{}.max_size() - 1;
  }

  // [[nodiscard]]
  constexpr bool empty() const noexcept {
    return *_data == '\0';
  }

  constexpr void remove_prefix(size_type n) {
    auto length = size();
    if (n >= length) {
      _data = empty_string;
      _length = 0;
    } else {
      _data += n;
      _length = length - n;
    }
  }

  constexpr void swap(basic_cstring_view &v) noexcept {
    basic_cstring_view temp{*this};
    *this = v;
    v = temp;
  }

  //constexpr
  size_type copy(CharT *dest, size_type count, size_type pos = 0) const {
    auto const length = size();
    if (pos >= length) {
      return 0;
    }
    if (count > length - pos) {
      count = length - pos;
    }
    return Traits::copy(dest, _data + pos, count);
  }

  // Copies count - 1 characters into dest and appends a null terminator.
  size_type c_copy(CharT *dest, size_type count, size_type pos = 0) const {
    auto const length = size();
    if (pos >= length) {
      return 0;
    }
    if (count > length - pos) {
      count = length - pos;
    }
    size_type copied;
    if (count == 0) {
      copied = 0;
    } else {
      copied = Traits::copy(dest, _data + pos, count - 1);
    }
    dest[copied] = 0;
    return copied + 1;
  }

  constexpr basic_cstring_view substr(size_type pos = 0) {
    auto const length = size();
    if (pos >= length) {
      return {};
    }
    return {_data + pos, length - pos};
  }

  constexpr int compare(basic_cstring_view v) const noexcept {
    auto sz0 = size();
    auto sz1 = v.size();
    if (sz0 == sz1) {
      return Traits::compare(_data, v._data, sz0);
    } else if (sz0 > sz1) {
      return int(sz0 - sz1);
    } else {
      return -int(sz1 - sz0);
    }
  }

#if 0
  constexpr int compare(size_type pos1, size_type count1,
                        basic_cstring_view v) const;
  constexpr int compare(size_type pos1, size_type count1, basic_cstring_view v,
                        size_type pos2, size_type count2) const;
#endif
  constexpr int compare(const CharT* s) const {
    return compare(basic_cstring_view{s});
  }
#if 0
  constexpr int compare(size_type pos1, size_type count1,
                        const CharT* s) const;
  constexpr int compare(size_type pos1, size_type count1,
                        const CharT* s, size_type count2) const;
#endif

  constexpr bool starts_with(basic_cstring_view sv) const noexcept {
    if (!_data) {
      return false;
    }
    for (size_t i = 0; ; ++i) {
      if (_data[i] != sv._data[i]) {
        return false;
      }
    }
    return true;
  }

  constexpr bool starts_with(CharT c) const noexcept {
    return _data && _data[0] == c;
  }

  constexpr bool starts_with(const CharT* s) const {
    return starts_with(basic_cstring_view<CharT>{s});
  }

#if 0
  constexpr bool ends_with(basic_cstring_view sv) const noexcept;
  constexpr bool ends_with(CharT c) const noexcept;
  constexpr bool ends_with(const CharT* s) const;

  constexpr bool contains(basic_cstring_view sv) const noexcept;
  constexpr bool contains(CharT c) const noexcept;
  constexpr bool contains(const CharT* s) const;

  constexpr size_type find(basic_cstring_view v,
                           size_type pos = 0) const noexcept;
  constexpr size_type find(CharT ch, size_type pos = 0) const noexcept;
  constexpr size_type find(const CharT* s, size_type pos,
                           size_type count) const;
  constexpr size_type find(const CharT* s, size_type pos = 0) const;

  constexpr size_type rfind(basic_cstring_view v,
                            size_type pos = npos) const noexcept;
  constexpr size_type rfind(CharT ch, size_type pos = npos) const noexcept;
  constexpr size_type rfind(const CharT* s, size_type pos,
                            size_type count) const;
  constexpr size_type rfind(const CharT* s, size_type pos = npos) const;

  constexpr size_type find_first_of(basic_string_view v,
                                    size_type pos = 0) const noexcept;
  constexpr size_type find_first_of(CharT c, size_type pos = 0) const noexcept;
  constexpr size_type find_first_of(const CharT* s, size_type pos,
                                    size_type count) const;
  constexpr size_type find_first_of(const CharT* s, size_type pos = 0) const;

  constexpr size_type find_last_of(basic_string_view v,
                                   size_type pos = npos) const noexcept;
  constexpr size_type find_last_of(CharT c,
                                   size_type pos = npos) const noexcept;
  constexpr size_type find_last_of(const CharT* s, size_type pos,
                                   size_type count) const;
  constexpr size_type find_last_of(const CharT* s,
                                   size_type pos = npos) const;

  constexpr size_type find_first_not_of(basic_string_view v,
                                        size_type pos = 0) const noexcept;
  constexpr size_type find_first_not_of(CharT c,
                                        size_type pos = 0) const noexcept;
  constexpr size_type find_first_not_of(const CharT* s, size_type pos,
                                        size_type count) const;
  constexpr size_type find_first_not_of(const CharT* s,
                                        size_type pos = 0) const;

  constexpr size_type find_last_not_of(basic_string_view v,
                                       size_type pos = npos) const noexcept;
  constexpr size_type find_last_not_of(CharT c,
                                       size_type pos = npos) const noexcept;
  constexpr size_type find_last_not_of(const CharT* s, size_type pos,
                                       size_type count) const;
  constexpr size_type find_last_not_of(const CharT* s,
                                       size_type pos = npos) const;
#endif

  explicit operator bool() const noexcept {
    return !empty();
  }

  constexpr operator const CharT *() const noexcept {
    return _data;
  }

  constexpr operator std::basic_string_view<CharT, Traits>() const noexcept {
    return {_data, size()};
  }

  constexpr explicit operator std::basic_string<CharT, Traits>() const {
    return {_data, size()};
  }

private:
  // Pointer to a null terminated string.
  const CharT *_data;

  // If this is `npos`, the length is not known. Length is computed on demand
  // the first time it is requested.
  mutable size_type _length;
};

template<class CharT, class Traits>
constexpr bool operator==(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return a.compare(b) == 0;
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
  return a.compare(b) != 0;
}

template<class CharT, class Traits>
constexpr bool operator<(basic_cstring_view<CharT, Traits> a,
                         basic_cstring_view<CharT, Traits> b) noexcept {
  return a.compare(b) < 0;
}

template<class CharT, class Traits>
constexpr bool operator<=(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return a.compare(b) <= 0;
}

template<class CharT, class Traits>
constexpr bool operator>(basic_cstring_view<CharT, Traits> a,
                         basic_cstring_view<CharT, Traits> b) noexcept {
  return a.compare(b) > 0;
}

template<class CharT, class Traits>
constexpr bool operator>=(basic_cstring_view<CharT, Traits> a,
                          basic_cstring_view<CharT, Traits> b) noexcept {
  return a.compare(b) >= 0;
}

#endif // __cplusplus > 201907L

using cstring_view = basic_cstring_view<char>;
using u8cstring_view = basic_cstring_view<
#if __cplusplus >= 201811L
  char8_t
#else
  char
#endif
  >;
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

template<class CharT, class Traits>
struct std::hash<a8::basic_cstring_view<CharT, Traits>> {
  constexpr size_t
  operator()(const a8::basic_cstring_view<CharT, Traits> &v) const noexcept {
    return hash<std::basic_string_view<CharT, Traits>>{}(v);
  }
};

#endif // A8_UTIL_CSTRING_VIEW_H_
