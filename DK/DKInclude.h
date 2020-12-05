//
//  File: DKInclude.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>

////////////////////////////////////////////////////////////////////////////////
// Build Configuration
#if defined(_DEBUG) || defined(DEBUG)
#   ifndef DKGL_DEBUG_ENABLED
#       define DKGL_DEBUG_ENABLED 1
#   endif
#   ifdef _MSC_VER
#       pragma message("Build DK with Debug Configuration.")
#   else
#       warning Build DK with Debug Configuration.
#   endif
#else
#   ifdef _MSC_VER
#       pragma message("Build DK with Release Configuration.")
#   else
#       warning Build DK with Release Configuration.
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////
// WIN32
#ifdef _WIN32
#   define DKGL_WIN32 1
#   include <SDKDDKVer.h>
#   if !defined(DKGL_STATIC) && !defined(DKGL_DYNAMIC)
#       define DKGL_DYNAMIC 1	// DLL is default on Win32
#   endif
#   ifdef DKGL_DYNAMIC			// DLL
#       pragma message("Build DK for Win32 DLL.")
#       ifdef DKGL_EXPORTS
#           define DKGL_API	__declspec(dllexport)
#       else
#           define DKGL_API	__declspec(dllimport)
#       endif
#   else						// static
#       pragma message("Build DK for Win32 Static Library.")
#       define DKGL_API
#   endif	// ifdef DKGL_DYNAMIC
#endif	// ifdef _WIN32

////////////////////////////////////////////////////////////////////////////////
// APPLE OSX, iOS
#if defined(__APPLE__) && defined(__MACH__)
#   define DKGL_APPLE_MACH
#   include <TargetConditionals.h>
#   if !defined(DKGL_STATIC) && !defined(DKGL_DYNAMIC)
#       define DKGL_DYNAMIC 1	// dylib(Framework) is default.
#   endif
#   if TARGET_OS_IPHONE
#       define DKGL_APPLE_IOS 1
#       ifdef DKGL_DYNAMIC		// dylib or Framework
#           define DKGL_API	__attribute__((visibility ("default")))
#           if TARGET_OS_SIMULATOR
#               warning Build DK iOS Dynamic Library for iOS Simulator.
#           else	//if TARGET_OS_SIMULATOR
#               warning Build DK iOS Dynamic Library for iOS Device.
#           endif	//if TARGET_OS_SIMULATOR
#       else
#           define DKGL_API
#           if TARGET_OS_SIMULATOR
#               warning Build DK iOS Static Library for iOS Simulator.
#           else	//if TARGET_OS_SIMULATOR
#               warning Build DK iOS Static Library for iOS Device.
#           endif	//if TARGET_OS_SIMULATOR
#       endif
#   else	//if TARGET_OS_IPHONE
#       define DKGL_APPLE_OSX 1
#       ifdef DKGL_DYNAMIC		// dylib or Framework
#           define DKGL_API	__attribute__((visibility ("default")))
#           warning Build DK for macOS Dynamic Library.
#       else
#           define DKGL_API
#           warning Build DK for macOS Static Library.
#       endif
#   endif
#endif //if defined(__APPLE__) && defined(__MACH__)

////////////////////////////////////////////////////////////////////////////////
// LINUX, ANDROID
#ifdef __linux__
#   include <sys/endian.h>
#   if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#       if __BYTE_ORDER == __LITTLE_ENDIAN
#           define __LITTLE_ENDIAN__ 1
#       elif __BYTE_ORDER == __BIG_ENDIAN
#           define __BIG_ENDIAN__ 1
#       endif
#   endif
#   define DKGL_LINUX 1

// no default library on Linux. You should define static or dynamic.
#   if !defined(DKGL_STATIC) && !defined(DKGL_DYNAMIC)
#       error "You should define DKGL_STATIC or DKGL_DYNAMIC"
#   endif

#   ifdef DKGL_DYNAMIC		// so
#       define DKGL_API	__attribute__((visibility ("default")))
#       warning Build DK for Linux Shared Library.
#   else
#       define DKGL_API
#       warning Build DK for Linux Static Library.
#   endif
#endif

// Unsupported OS?
#if !defined(DKGL_STATIC) && !defined(DKGL_DYNAMIC)
#   error "You should define DKGL_STATIC or DKGL_DYNAMIC"
#endif

// Macros for byte order conversion.
// byte order should be defined, It will be verified at run-time.
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#   if defined(_M_X64) || defined(_M_IX86) || defined(__i386__)
#       define __LITTLE_ENDIAN__	1
#   elif defined(__ARMEB__)
#       define __BIG_ENDIAN__ 1
#   else
#       error "System byte order not defined."
#   endif
#endif

// Inline macros
#ifndef FORCEINLINE
#   ifdef DKGL_DEBUG_ENABLED
#       define FORCEINLINE inline
#   else
#       ifdef _MSC_VER
#           define FORCEINLINE __forceinline
#       else
#           define FORCEINLINE inline __attribute__((always_inline))
#       endif
#   endif
#endif
#ifndef NOINLINE
#   ifdef _MSC_VER
#       define NOINLINE __declspec(noinline)
#   else
#       define NOINLINE __attribute__((noinline))
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////
// useful templates (C++ only)
#ifdef __cplusplus
#	include <stdexcept>
#	include <type_traits>
namespace DKFoundation
{
    template <typename T> using _UnRef = typename std::remove_reference<T>::type;
    template <typename T> using _UnCV = typename std::remove_cv<T>::type;
    template <typename T> using _UnRefCV = _UnCV<_UnRef<T>>;

    // Min
    template <typename T> FORCEINLINE auto Min(T&& lhs, T&& rhs)->T&&
    {
        return std::forward<T>((lhs < rhs) ? lhs : rhs);
    }
    template <typename T, typename U> FORCEINLINE auto Min(T&& lhs, U&& rhs)->_UnRefCV<T>
    {
        return static_cast<_UnRef<T>>((lhs < rhs) ? lhs : rhs);
    }
    template <typename T, typename U, typename... V> FORCEINLINE auto Min(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
    {
        return Min(std::forward<T>(v1), Min(std::forward<U>(v2), std::forward<V>(rest)...));
    }

    // Max
    template <typename T> FORCEINLINE auto Max(T&& lhs, T&& rhs)->T&&
    {
        return std::forward<T>((lhs > rhs) ? lhs : rhs);
    }
    template <typename T, typename U> FORCEINLINE auto Max(T&& lhs, U&& rhs)->_UnRefCV<T>
    {
        return static_cast<_UnRef<T>>(((lhs > rhs) ? lhs : rhs));
    }
    template <typename T, typename U, typename... V> FORCEINLINE auto Max(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
    {
        return Max(std::forward<T>(v1), Max(std::forward<U>(v2), std::forward<V>(rest)...));
    }

    // Clamp
    template <typename T> FORCEINLINE auto Clamp(T&& v, T&& _min, T&& _max)->T&&
    {
        return Min(Max(std::forward<T>(v), std::forward<T>(_min)), std::forward<T>(_max));
    }
    template <typename T, typename MinT, typename MaxT> FORCEINLINE auto Clamp(T&& v, MinT&& _min, MaxT&& _max)->_UnRefCV<T>
    {
        return Min(Max(std::forward<T>(v), std::forward<MinT>(_min)), std::forward<MaxT>(_max));
    }

    DKGL_API void DKErrorRaiseException(const char*, const char*, unsigned int, const char*);
}
namespace DKFramework
{
    using namespace DKFoundation;
}
////////////////////////////////////////////////////////////////////////////////
// Macros for error, exception (DKError), C++ only.

#   if defined(_MSC_VER)
#       define DKGL_FUNCTION_NAME       __FUNCTION__
#   elif defined(__GNUC__)
#       define DKGL_FUNCTION_NAME       __PRETTY_FUNCTION__
#   else
#       define DKGL_FUNCTION_NAME       __func__
#   endif

#   define DKGL_NOOP                        (void)0 // Forces the macro to end the statement with a semicolon.
#   define DKERROR_THROW(desc)              DKFoundation::DKErrorRaiseException(DKGL_FUNCTION_NAME, __FILE__, __LINE__, desc)
#   define DKASSERT_DESC(expr, desc)        {if (!(expr)) DKERROR_THROW(desc);}             DKGL_NOOP
#   define DKASSERT(expr)                   {if (!(expr)) DKERROR_THROW("");}               DKGL_NOOP
#   define DKASSERT_STD_DESC(expr, desc)    {if (!(expr)) throw std::runtime_error(desc);}  DKGL_NOOP
#   define DKASSERT_STD(expr)               {if (!(expr)) throw std::runtime_error("");}    DKGL_NOOP

#   ifdef DKGL_DEBUG_ENABLED
#       define DKERROR_THROW_DEBUG(desc)            DKERROR_THROW(desc)
#       define DKASSERT_DESC_DEBUG(expr, desc)      DKASSERT_DESC(expr, desc)
#       define DKASSERT_DEBUG(expr)                 DKASSERT(expr)
#       define DKASSERT_STD_DESC_DEBUG(expr, desc)  DKASSERT_STD_DESC(expr, desc)
#       define DKASSERT_STD_DEBUG(expr)             DKASSERT_STD(expr)
#   else
#       define DKERROR_THROW_DEBUG(desc)            DKGL_NOOP
#       define DKASSERT_DESC_DEBUG(expr, desc)      DKGL_NOOP
#       define DKASSERT_DEBUG(expr)                 DKGL_NOOP
#       define DKASSERT_STD_DESC_DEBUG(expr, desc)  DKGL_NOOP
#       define DKASSERT_STD_DEBUG(expr)             DKGL_NOOP
#	endif

#   ifndef DKGL_MEMORY_DEBUG
#       ifdef DKGL_DEBUG_ENABLED
#           define DKGL_MEMORY_DEBUG 1
#       else
#           define DKGL_MEMORY_DEBUG 0
#       endif
#	endif /* DKGL_MEMORY_DEBUG */
#   if DKGL_MEMORY_DEBUG
#       define DKASSERT_MEM_DESC(expr, desc)        {if (!(expr)) throw std::runtime_error(desc);}  DKGL_NOOP
#       define DKASSERT_MEM(expr)                   {if (!(expr)) throw std::runtime_error("");}    DKGL_NOOP
#       define DKASSERT_MEM_DESC_DEBUG(expr, desc)  DKASSERT_STD_DESC(expr, desc)
#       define DKASSERT_MEM_DEBUG(expr)             DKASSERT_STD(expr)
#   else
#       define DKASSERT_MEM_DESC(expr, desc)        DKGL_NOOP
#       define DKASSERT_MEM(expr)                   DKGL_NOOP
#       define DKASSERT_MEM_DESC_DEBUG(expr, desc)  DKGL_NOOP
#       define DKASSERT_MEM_DEBUG(expr)             DKGL_NOOP
#   endif
#endif	// #ifdef __cplusplus

////////////////////////////////////////////////////////////////////////////////
// DK framework info
#ifdef __cplusplus
extern "C"
{
#endif
    DKGL_API const char* DKVersion(void);
    DKGL_API const char* DKCopyright(void);
#ifdef __cplusplus
}
#endif
