#pragma once


#ifdef PYDK_EXPORTS
#  ifndef PYDK_DYNAMIC
#    define PYDK_DYNAMIC
#  endif
#else
#  if !defined(PYDK_STATIC) && !defined(PYDK_DYNAMIC)
#    define PYDK_DYNAMIC     /* dynamic-library is default. */
#  endif
#endif


#if defined(PYDK_DYNAMIC)
#  ifdef _MSC_VER
#    ifdef PYDK_EXPORTS
#      define PYDK_API	__declspec(dllexport)
#    else /* PYDK_EXPORTS */
#      define PYDK_API __declspec(dllimport)
#    endif /* PYDK_EXPORTS */
#  else  /* _MSC_VER */
#    define PYDK_API __attribute__((visibility ("default")))
#  endif
#elif defined(PYDK_STATIC)
#  define PYDK_API
#else
#error "You should define PYDK_STATIC or PYDK_DYNAMIC."
#endif
