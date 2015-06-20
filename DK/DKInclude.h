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
#	ifndef DKLIB_DEBUG_ENABLED
#		define DKLIB_DEBUG_ENABLED 1
#	endif
#	ifdef _MSC_VER
#		pragma message("Build DK with Debug Configuration.")
#	else
#		warning Build DK with Debug Configuration.
#	endif
#else
#	ifdef _MSC_VER
#		pragma message("Build DK with Release Configuration.")
#	else
#		warning Build DK with Release Configuration.
#	endif
#endif

////////////////////////////////////////////////////////////////////////////////
// WIN32
#ifdef _WIN32
#	define DKLIB_WIN32 1
#	include <SDKDDKVer.h>
#	if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
#		define DKLIB_DYNAMIC 1	// DLL is default on Win32
#	endif
#	ifdef DKLIB_DYNAMIC			// DLL
#		pragma message("Build DK for Win32 DLL.")
#		ifdef DKLIB_EXPORTS
#			define DKLIB_API	__declspec(dllexport)
#		else
#			define DKLIB_API	__declspec(dllimport)
#		endif
#	else						// static
#		pragma message("Build DK for Win32 Static Library.")
#		define DKLIB_API
#	endif	// ifdef DKLIB_DYNAMIC
#endif	// ifdef _WIN32

////////////////////////////////////////////////////////////////////////////////
// APPLE OSX, iOS
#if defined(__APPLE__) && defined(__MACH__)
#	define DKLIB_APPLE_MACH
#	include <TargetConditionals.h>
#	if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
#		define DKLIB_DYNAMIC 1	// dylib(Framework) is default.
#	endif
#	if TARGET_OS_IPHONE
#		define DKLIB_APPLE_IOS 1
#		ifdef DKLIB_DYNAMIC		// dylib or Framework
#			define DKLIB_API	__attribute__((visibility ("default")))
#			if TARGET_IPHONE_SIMULATOR
#				warning Build DK iOS Dynamic Library for iOS Simulator.
#			else	//if TARGET_IPHONE_SIMULATOR
#				warning Build DK iOS Dynamic Library for iOS Device.
#			endif	//if TARGET_IPHONE_SIMULATOR
#		else
#			define DKLIB_API
#			if TARGET_IPHONE_SIMULATOR
#				warning Build DK iOS Static Library for iOS Simulator.
#			else	//if TARGET_IPHONE_SIMULATOR
#				warning Build DK iOS Static Library for iOS Device.
#			endif	//if TARGET_IPHONE_SIMULATOR
#		endif
#	else	//if TARGET_OS_IPHONE
#		define DKLIB_APPLE_OSX 1
#		ifdef DKLIB_DYNAMIC		// dylib or Framework
#			define DKLIB_API	__attribute__((visibility ("default")))
#			warning Build DK for Mac OS X Dynamic Library.
#		else
#			define DKLIB_API
#			warning Build DK for Mac OS X Static Library.
#		endif
#	endif
#endif //if defined(__APPLE__) && defined(__MACH__)

////////////////////////////////////////////////////////////////////////////////
// LINUX, ANDROID
#ifdef __linux__
#	include <sys/endian.h>
#	if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#		if __BYTE_ORDER == __LITTLE_ENDIAN
#			define __LITTLE_ENDIAN__ 1
#		elif __BYTE_ORDER == __BIG_ENDIAN
#			define __BIG_ENDIAN__ 1
#		endif
#	endif
#	define DKLIB_LINUX 1

// no default library on Linux. You should define static or dynamic.
#	if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
#		error "You should define DKLIB_STATIC or DKLIB_DYNAMIC"
#	endif

#	ifdef DKLIB_DYNAMIC		// so
#		define DKLIB_API	__attribute__((visibility ("default")))
#		warning Build DK for Linux Shared Library.
#	else
#		define DKLIB_API
#		warning Build DK for Linux Static Library.
#	endif
#endif

// Unsupported OS?
#if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
#	error "You should define DKLIB_STATIC or DKLIB_DYNAMIC"
#endif

// Macros for byte order conversion.
// byte order should be defined, It will be verified at run-time.
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#	if defined(_M_X64) || defined(_M_IX86) || defined(__i386__)
#		define __LITTLE_ENDIAN__	1
#	elif defined(__ARMEB__)
#		define __BIG_ENDIAN__ 1
#	else
#		error "System byte order not defined."
#	endif
#endif

// Inline macros
#ifndef FORCEINLINE
#	ifdef DKLIB_DEBUG_ENABLED
#		define FORCEINLINE
#	else
#		ifdef _MSC_VER
#			define FORCEINLINE __forceinline
#		else
#			define FORCEINLINE __attribute__((always_inline))
#		endif
#	endif
#endif
#ifndef NOINLINE
#	ifdef _MSC_VER
#		define NOINLINE __declspec(noinline)
#	else
#		define NOINLINE __attribute__((noinline))
#	endif
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
	template <typename T> auto Min(T&& lhs, T&& rhs)->T&&
	{
		return std::forward<T>((lhs < rhs) ? lhs : rhs);
	}
	template <typename T, typename U> auto Min(T&& lhs, U&& rhs)->_UnRefCV<T>
	{
		return static_cast<_UnRef<T>>((lhs < rhs) ? lhs : rhs);
	}
	template <typename T, typename U, typename... V> auto Min(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
	{
		return Min(std::forward<T>(v1), Min(std::forward<U>(v2), std::forward<V>(rest)...));
	}

	// Max
	template <typename T> auto Max(T&& lhs, T&& rhs)->T&&
	{
		return std::forward<T>((lhs > rhs) ? lhs : rhs);
	}
	template <typename T, typename U> auto Max(T&& lhs, U&& rhs)->_UnRefCV<T>
	{
		return static_cast<_UnRef<T>>(((lhs > rhs) ? lhs : rhs));
	}
	template <typename T, typename U, typename... V> auto Max(T&& v1, U&& v2, V&&... rest)->_UnRefCV<T>
	{
		return Max(std::forward<T>(v1), Max(std::forward<U>(v2), std::forward<V>(rest)...));
	}

	// Clamp
	template <typename T> auto Clamp(T&& v, T&& _min, T&& _max)->T&&
	{
		return Min(Max(std::forward<T>(v), std::forward<T>(_min)), std::forward<T>(_max));
	}
	template <typename T, typename MinT, typename MaxT> auto Clamp(T&& v, MinT&& _min, MaxT&& _max)->_UnRefCV<T>
	{
		return Min(Max(std::forward<T>(v), std::forward<MinT>(_min)), std::forward<MaxT>(_max));
	}

	DKLIB_API void DKErrorRaiseException(const char*, const char*, unsigned int, const char*);
}

////////////////////////////////////////////////////////////////////////////////
// Macros for error, exception (DKError), C++ only.

#	if defined(_MSC_VER)
#		define DKLIB_FUNCTION_NAME		__FUNCTION__
#	elif defined(__GNUC__)
#		define DKLIB_FUNCTION_NAME		__PRETTY_FUNCTION__
#	else
#		define DKLIB_FUNCTION_NAME		__func__
#	endif

#	define DKERROR_THROW(desc)				DKFoundation::DKErrorRaiseException(DKLIB_FUNCTION_NAME, __FILE__, __LINE__, desc)
#	define DKASSERT_DESC(expr, desc)		{if (!(expr)) DKERROR_THROW(desc);}
#	define DKASSERT(expr)					{if (!(expr)) DKERROR_THROW("");}
#	define DKASSERT_STD_DESC(expr, desc)	{if (!(expr)) throw std::runtime_error(desc);}
#	define DKASSERT_STD(expr)				{if (!(expr)) throw std::runtime_error("");}

#	ifdef DKLIB_DEBUG_ENABLED
#		define DKERROR_THROW_DEBUG(desc)			DKERROR_THROW(desc)
#		define DKASSERT_DESC_DEBUG(expr, desc)		DKASSERT_DESC(expr, desc)
#		define DKASSERT_DEBUG(expr)					DKASSERT(expr)
#		define DKASSERT_STD_DESC_DEBUG(expr, desc)	DKASSERT_STD_DESC(expr, desc)
#		define DKASSERT_STD_DEBUG(expr)				DKASSERT_STD(expr)
#	else
#		define DKERROR_THROW_DEBUG(desc)			(void)0
#		define DKASSERT_DESC_DEBUG(expr, desc)		(void)0
#		define DKASSERT_DEBUG(expr)					(void)0
#		define DKASSERT_STD_DESC_DEBUG(expr, desc)	(void)0
#		define DKASSERT_STD_DEBUG(expr)				(void)0
#	endif

#	ifndef DKLIB_MEMORY_DEBUG
#		ifdef DKLIB_DEBUG_ENABLED
#			define DKLIB_MEMORY_DEBUG 1
#		else
#			define DKLIB_MEMORY_DEBUG 0
#		endif
#	endif /* DKLIB_MEMORY_DEBUG */
#	if DKLIB_MEMORY_DEBUG
#		define DKASSERT_MEM_DESC(expr, desc)		{if (!(expr)) throw std::runtime_error(desc);}
#		define DKASSERT_MEM(expr)					{if (!(expr)) throw std::runtime_error("");}
#		define DKASSERT_MEM_DESC_DEBUG(expr, desc)	DKASSERT_STD_DESC(expr, desc)
#		define DKASSERT_MEM_DEBUG(expr)				DKASSERT_STD(expr)
#	else
#		define DKASSERT_MEM_DESC(expr, desc)		(void)0
#		define DKASSERT_MEM(expr)					(void)0
#		define DKASSERT_MEM_DESC_DEBUG(expr, desc)	(void)0
#		define DKASSERT_MEM_DEBUG(expr)				(void)0
#	endif
#endif	// #ifdef __cplusplus

////////////////////////////////////////////////////////////////////////////////
// DK framework info
#ifdef __cplusplus
extern "C"
{
#endif
	DKLIB_API const char* DKVersion(void);
	DKLIB_API const char* DKCopyright(void);
#ifdef __cplusplus
}
#endif
