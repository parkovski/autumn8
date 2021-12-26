#ifndef A8_UTIL_URI_H_
#define A8_UTIL_URI_H_

#include "cstring_view.h"

#include <uriparser/Uri.h>

#include <string_view>
#include <utility>
#include <type_traits>
#include <memory>

namespace a8 {

template<class UriT>
struct UriTraits;

#define A8_DEFINE_URI_FUNCTIONS(Suffix)                                       \
  static inline int                                                           \
  parseSingleUriEx(UriUri##Suffix *uri, const char_type *first,               \
                   const char_type *afterLast, const char_type **errorPos) {  \
    int error = uriParseSingleUriEx##Suffix(uri, first, afterLast, errorPos); \
    if (error == URI_ERROR_MALLOC) {                                          \
      throw std::bad_alloc{};                                                 \
    }                                                                         \
    return error;                                                             \
  }                                                                           \
                                                                              \
  static inline void makeOwner(UriUri##Suffix *uri) {                         \
    if (uriMakeOwner##Suffix(uri) == URI_ERROR_MALLOC) {                      \
      throw std::bad_alloc{};                                                 \
    }                                                                         \
  }                                                                           \
                                                                              \
  static inline void freeUriMembers(UriUri##Suffix *uri) noexcept {           \
    uriFreeUriMembers##Suffix(uri);                                           \
  }                                                                           \
                                                                              \
  static inline void                                                          \
  dissectQueryMalloc(UriQueryList##Suffix **list, size_t *itemCount,          \
                     UriTextRange##Suffix range) {                            \
    int iitemCount;                                                           \
    if (uriDissectQueryMalloc##Suffix(list, &iitemCount, range.first,         \
                                      range.afterLast) == URI_ERROR_MALLOC) { \
      throw std::bad_alloc{};                                                 \
    }                                                                         \
    *itemCount = static_cast<size_t>(iitemCount);                             \
  }                                                                           \
                                                                              \
  static inline void freeQueryList(UriQueryList##Suffix *list) {              \
    uriFreeQueryList##Suffix(list);                                           \
  }                                                                           \
                                                                              \
  static inline constexpr std::basic_string_view<char_type>                   \
  to_string_view(UriTextRange##Suffix range) noexcept {                       \
    return {range.first, static_cast<size_t>(range.afterLast - range.first)}; \
  }


#if URI_ENABLE_ANSI
template<>
struct UriTraits<UriUriA> {
  using char_type = char;
  using text_range_type = UriTextRangeA;
  using path_segment_type = UriPathSegmentA;
  using query_list_type = UriQueryListA;

  A8_DEFINE_URI_FUNCTIONS(A)
};
#endif // URI_ENABLE_ANSI

#if URI_ENABLE_UNICODE
template<>
struct UriTraits<UriUriW> {
  using char_type = wchar_t;
  using text_range_type = UriTextRangeW;
  using path_segment_type = UriPathSegmentW;
  using query_list_type = UriQueryListW;

  A8_DEFINE_URI_FUNCTIONS(W)
};
#endif // URI_ENABLE_UNICODE

#undef A8_DEFINE_URI_FUNCTIONS

// NB: Modifying the pointers in this class will modify the URI that they
// come from.
template<class UriT, class Traits = UriTraits<UriT>>
struct basic_uri_path {
private:
  typename Traits::path_segment_type _head;
  typename Traits::path_segment_type _tail;

public:
  explicit basic_uri_path(typename Traits::path_segment_type head,
                          typename Traits::path_segment_type tail) noexcept
    : _head{head},
      _tail{tail}
  {}

  basic_uri_path(const basic_uri_path &) = default;
  basic_uri_path &operator=(const basic_uri_path &) = default;

  bool operator==(const basic_uri_path &other) const noexcept {
    return _head == other._head && _tail == other._tail;
  }

  bool operator!=(const basic_uri_path &other) const noexcept {
    return !(*this == other);
  }

  std::basic_string_view<typename Traits::char_type> str() const noexcept {
    return {
      _head.text.first,
      static_cast<size_t>(_tail.text.afterLast - _head.text.first)
    };
  }

  operator std::basic_string_view<typename Traits::char_type>() const noexcept
  {
    return str();
  }

  struct const_iterator {
    typedef void difference_type;
    typedef std::basic_string_view<typename Traits::char_type> value_type;
    typedef value_type reference;
    typedef void pointer;
    typedef std::input_iterator_tag iterator_category;

  private:
    typename Traits::path_segment_type _seg;

  public:
    const_iterator() noexcept
      : _seg{}
    {}

    /// \param seg URI path segment, must not be null!
    const_iterator(typename Traits::path_segment_type seg) noexcept
      : _seg{seg}
    {}

    reference operator*() const noexcept {
      return Traits::to_string_view(_seg.text);
    }

    const_iterator &operator++() noexcept {
      if (_seg.next) {
        _seg = *_seg.next;
      } else {
        _seg.next = nullptr;
        _seg.text = {nullptr, nullptr};
      }
      return *this;
    }

    const_iterator operator++(int) noexcept {
      const_iterator current = *this;
      ++*this;
      return current;
    }

    bool operator==(const const_iterator &other) const noexcept {
      return _seg.text.first == other._seg.text.first &&
        _seg.text.afterLast == other._seg.text.afterLast;
    }

    bool operator!=(const const_iterator &other) const noexcept {
      return !(*this == other);
    }
  };
  using iterator = const_iterator;

  const_iterator cbegin() const noexcept {
    // If the path started with a '/', uriparser doesn't include it, but
    // we want it included in the full path for str() so need to correct for
    // that here.
    auto realHead = _head;
    if (realHead.text.first != realHead.text.afterLast &&
        *realHead.text.first == '/') {
      ++realHead.text.first;
    }
    return realHead;
  }

  const_iterator begin() const noexcept {
    return cbegin();
  }

  const_iterator cend() const noexcept {
    return {};
  }

  const_iterator end() const noexcept {
    return cend();
  }
};

template<class UriT, class Traits = UriTraits<UriT>>
struct basic_uri_query_list {
private:
  typename Traits::query_list_type *_list;
  size_t _size;

public:
  explicit basic_uri_query_list(typename Traits::text_range_type range) {
    if (!range.first) {
      _list = nullptr;
      _size = 0;
    } else {
      Traits::dissectQueryMalloc(&_list, &_size, range);
    }
  }

  basic_uri_query_list(basic_uri_query_list &&other) noexcept
    : _list{other._list},
      _size{other._size}
  {
    other._list = nullptr;
  }

  basic_uri_query_list &operator=(basic_uri_query_list &&other) noexcept {
    if (_list) {
      Traits::freeQueryList(_list);
    }
    _list = other._list;
    _size = other._size;
    other._list = nullptr;
    return *this;
  }

  basic_uri_query_list(const basic_uri_query_list &) = delete;
  basic_uri_query_list &operator=(const basic_uri_query_list &) = delete;

  ~basic_uri_query_list() {
    if (_list) {
      Traits::freeQueryList(_list);
    }
  }

  friend void swap(basic_uri_query_list &a, basic_uri_query_list &b) noexcept {
    auto a_list = a._list;
    a._list = b._list;
    b._list = a_list;
  }

  size_t size() const noexcept {
    return _size;
  }

  struct const_iterator {
    typedef void difference_type;
    typedef std::pair<
      basic_cstring_view<typename Traits::char_type>,
      basic_cstring_view<typename Traits::char_type>> value_type;
    typedef void pointer;
    typedef value_type reference;
    typedef std::input_iterator_tag iterator_category;

  private:
    typename Traits::query_list_type const *_list;

  public:
    const_iterator() noexcept : _list{nullptr} {}
    const_iterator(typename Traits::query_list_type const *list)
      : _list{list}
    {}

    reference operator*() const noexcept {
      return {_list->key, _list->value};
    }

    const_iterator &operator++() noexcept {
      _list = _list->next;
      return *this;
    }

    const_iterator operator++(int) noexcept {
      const_iterator current = *this;
      ++*this;
      return current;
    }

    bool operator==(const const_iterator &other) const noexcept {
      return _list == other._list;
    }

    bool operator!=(const const_iterator &other) const noexcept {
      return !(*this == other);
    }
  };
  using iterator = const_iterator;

  const_iterator cbegin() const noexcept {
    // OK if null - creates an end iterator.
    return _list;
  }

  const_iterator begin() const noexcept {
    return cbegin();
  }

  const_iterator cend() const noexcept {
    return {};
  }

  const_iterator end() const noexcept {
    return cend();
  }
};

template<class UriT, class Traits = UriTraits<UriT>>
struct basic_uri {
private:
  UriT _uri;

public:
  basic_uri() noexcept
    : _uri{}
  {}

  basic_uri(std::basic_string_view<typename Traits::char_type> str)
    : _uri{}
  {
    typename Traits::char_type const *errorPos;
    switch (Traits::parseSingleUriEx(&_uri, str.data(),
                                     str.data() + str.length(), &errorPos)) {
      case URI_SUCCESS:
        break;

      case URI_ERROR_SYNTAX:
        // errorPos is defined here.
        return;

      case URI_ERROR_MALLOC:
        throw std::bad_alloc{};
        return;

      case URI_ERROR_NULL:
        // Should only happen if str was empty I think.
        return;
    }

    // If the last segment is empty, the path ends in a '/', but unfortunately
    // uriparser sets the pointers to a different string. Change this back so
    // we can extract the full path without looping through the path list in
    // the future.
    if (_uri.pathTail &&
        _uri.pathTail->text.first == _uri.pathTail->text.afterLast) {
      if (_uri.pathHead == _uri.pathTail) {
        // The path consists of a single '/'. We didn't get any pointers to the
        // actual path part of the string so just fake it.
        _uri.pathHead->text.first = _uri.pathHead->text.afterLast =
          _uri.pathTail->text.first = _uri.pathTail->text.afterLast = "/";
      } else {
        auto *p = _uri.pathHead;
        while (p->next != _uri.pathTail) {
          p = p->next;
        }
        _uri.pathTail->text.first = _uri.pathTail->text.afterLast =
          p->text.afterLast + 1;
      }
    }
  }

  basic_uri(const basic_uri &other)
    : _uri{other._uri}
  {
    _uri.owner = false;
    if (Traits::makeOwner(&_uri) == URI_ERROR_MALLOC) {
      throw std::bad_alloc{};
    }
  }

  basic_uri(basic_uri &&other) noexcept
    : _uri{other._uri}
  {
    other._uri = {};
  }

  basic_uri &operator=(const basic_uri &other) {
    if (&other == this) {
      return *this;
    }

    Traits::freeUriMembers(&_uri);
    _uri = other._uri;
    _uri.owner = false;
    if (Traits::makeOwner(&_uri) == URI_ERROR_MALLOC) {
      throw std::bad_alloc{};
    }
    return *this;
  }

  basic_uri &operator=(basic_uri &&other) noexcept {
    if (&other == this) {
      return *this;
    }

    Traits::freeUriMembers(&_uri);
    _uri = other._uri;
    other._uri = {};
    return *this;
  }

  ~basic_uri() {
    Traits::freeUriMembers(&_uri);
  }

  friend void swap(basic_uri &a, basic_uri &b) noexcept {
    UriT a_uri = a._uri;
    a._uri = b._uri;
    b._uri = a_uri;
  }

  explicit operator bool() const noexcept {
    return _uri.scheme.first || _uri.hostText.first || _uri.pathHead ||
      _uri.query.first || _uri.fragment.first;
  }

  bool isAbsolute() const noexcept {
    return _uri.absolutePath;
  }

  std::basic_string_view<typename Traits::char_type> scheme() const noexcept {
    return Traits::to_string_view(_uri.scheme);
  }

  std::basic_string_view<typename Traits::char_type>
  userInfo() const noexcept {
    return Traits::to_string_view(_uri.userInfo);
  }

  std::basic_string_view<typename Traits::char_type> host() const noexcept {
    return Traits::to_string_view(_uri.hostText);
  }

  std::basic_string_view<typename Traits::char_type>
  portText() const noexcept {
    return Traits::to_string_view(_uri.portText);
  }

  basic_uri_path<UriT, Traits> path() const noexcept {
    if (!_uri.pathHead) {
      return basic_uri_path<UriT, Traits>{{}, {}};
    }
    auto headFull = *_uri.pathHead;
    auto tailFull = *_uri.pathTail;
    if (_uri.hostText.first || isAbsolute()) {
      if (headFull.text.first == tailFull.text.afterLast) {
        // The only character in the path is a '/'. Move the end of the path up
        // to include it, which will be cut off by the path iterator.
        tailFull.text.first = tailFull.text.afterLast =
          ++headFull.text.afterLast;
      } else {
        // The path has a leading '/' that was cut off.
        --headFull.text.first;
      }
    }
    return basic_uri_path<UriT, Traits>{headFull, tailFull};
  }

  std::basic_string_view<typename Traits::char_type>
  queryText() const noexcept {
    return Traits::to_string_view(_uri.query);
  }

  std::basic_string_view<typename Traits::char_type>
  fragment() const noexcept {
    return Traits::to_string_view(_uri.fragment);
  }
};

#if URI_ENABLE_ANSI
using Uri = basic_uri<UriUriA>;
#endif
#if URI_ENABLE_UNICODE
using UriW = basic_uri<UriUriW>;
#endif

} // namespace a8

#endif // A8_UTIL_URI_H_
