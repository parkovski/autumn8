// Compiler-specific feature support

#ifndef A8_COMPILER_H_
#define A8_COMPILER_H_

#if _WIN32
#  define A8_PRIVATE
#  if defined(__GNUC__) && !defined(__clang__)
#    define A8_IMPORT __attribute__((dllimport))
#    define A8_EXPORT __attribute__((dllexport))
#  else
#    define A8_IMPORT __declspec(dllimport)
#    define A8_EXPORT __declspec(dllexport)
#    if !defined(_MSC_VER)
#      warning "Unknown compiler; assuming MSVC-like."
#    endif
#  endif
#else
#  define A8_IMPORT __attribute__((visibility("default")))
#  define A8_EXPORT __attribute__((visibility("default")))
#  define A8_PRIVATE __attribute__((visibility("hidden")))
#  if !defined(__GNUC__) && !defined(__clang__)
#    warning "Unknown compiler; assuming GCC-like."
#  endif
#endif

#if A8_STATIC
#  define A8_PUBLIC
#elif BUILDING_DLL
#  define A8_PUBLIC A8_EXPORT
#else
#  define A8_PUBLIC A8_IMPORT
#endif

#ifdef _MSC_VER
#  define A8_PRAGMA_MS(s) __pragma(s)
#  define A8_PRAGMA(s) __pragma(s)
#  define A8_ATTR_MS(a) __declspec(a)
#  define A8_UNREACHABLE __assume(0)
#else
#  define A8_PRAGMA_MS(s)
#  define A8_ATTR_MS(a)
#
#  ifndef __has_builtin
#    define __has_builtin(b) 0
#    define A8_PLEASE_UNDEFINE_HAS_BUILTIN
#  endif
#
#  if __has_builtin(__builtin_unreachable)
#    define A8_UNREACHABLE __builtin_unreachable()
#  else
#    define A8_UNREACHABLE ((void)0)
#  endif
#
#  ifdef A8_PLEASE_UNDEFINE_HAS_BUILTIN
#    undef __has_builtin
#    undef A8_PLEASE_UNDEFINE_HAS_BUILTIN
#  endif
#endif

#ifdef __GNUC__
#  define A8_PRAGMA_GCC_CLANG(s) _Pragma(#s)
#  define A8_ATTR_GCC_CLANG(a) __attribute__((a))
#
#  ifdef __clang__
#    define A8_PRAGMA_CLANG(s) _Pragma(#s)
#    define A8_ATTR_CLANG(a) __attribute__((a))
#
#    define A8_PRAGMA_GCC(s)
#    define A8_ATTR_GCC(a)
#  else
#    define A8_PRAGMA_CLANG(s)
#    define A8_ATTR_CLANG(a)
#
#    define A8_PRAGMA_GCC(s) _Pragma(#s)
#    define A8_ATTR_GCC(a) __attribute__((a))
#  endif
#else
#  define A8_PRAGMA_GCC_CLANG(s)
#  define A8_ATTR_GCC_CLANG(a)
#  define A8_PRAGMA_GCC(s)
#  define A8_ATTR_GCC(a)
#  define A8_PRAGMA_CLANG(s)
#  define A8_ATTR_CLANG(s)
#endif

#ifndef A8_PRAGMA
#  define A8_PRAGMA _Pragma(#s)
#endif

#endif /* A8_COMPILER_H_ */
