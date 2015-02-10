//
//  File: DKInclude.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
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
	#ifndef DKLIB_DEBUG_ENABLED
		#define DKLIB_DEBUG_ENABLED
	#endif
	#ifdef _MSC_VER
		#pragma message("Build DK with Debug Configuration.")
	#else
		#warning Build DK with Debug Configuration.
	#endif
#else
	#ifdef _MSC_VER
		#pragma message("Build DK with Release Configuration.")
	#else
		#warning Build DK with Release Configuration.
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// WIN32
#ifdef _WIN32
	#define DKLIB_WIN32
	#include <SDKDDKVer.h>

	// DLL is default on Win32
	#if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
		#define DKLIB_DYNAMIC
	#endif

	#ifdef DKLIB_DYNAMIC		// dll
		#pragma message("Build DK for Win32 DLL.")
		#ifdef DKLIB_EXPORTS
			#define DKLIB_API	__declspec(dllexport)
		#else
			#define DKLIB_API	__declspec(dllimport)
		#endif
	#else						// static
		#pragma message("Build DK for Win32 Static Library.")
		#define DKLIB_API
	#endif	// ifdef DKLIB_DYNAMIC
	#ifdef PostMessage
	#undef PostMessage
	#endif
	#ifdef GetMessage
	#undef GetMessage
	#endif
	#ifdef Yield
	#undef Yield
	#endif
	#ifdef FindResource
	#undef FindResource
	#endif
	#ifdef RemoveDirectory
	#undef RemoveDirectory
	#endif
#endif	// ifdef _WIN32

////////////////////////////////////////////////////////////////////////////////
// APPLE OSX, iOS
#if defined(__APPLE__) && defined(__MACH__)
	#define DKLIB_APPLE_MACH
	#include <TargetConditionals.h>

	// dylib(Framework) is default.
	#if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
		#define DKLIB_DYNAMIC
	#endif

	#if TARGET_OS_IPHONE
		#define DKLIB_APPLE_IOS

		#ifdef DKLIB_DYNAMIC		// dylib or Framework
			#define DKLIB_API	__attribute__((visibility ("default")))
			#if TARGET_IPHONE_SIMULATOR
				#warning Build DK iOS Dynamic Library for iOS Simulator.
			#else	//if TARGET_IPHONE_SIMULATOR
				#warning Build DK iOS Dynamic Library for iOS Device.
			#endif	//if TARGET_IPHONE_SIMULATOR
		#else
			#define DKLIB_API
			#if TARGET_IPHONE_SIMULATOR
				#warning Build DK iOS Static Library for iOS Simulator.
			#else	//if TARGET_IPHONE_SIMULATOR
				#warning Build DK iOS Static Library for iOS Device.
			#endif	//if TARGET_IPHONE_SIMULATOR
		#endif
	#else	//if TARGET_OS_IPHONE
		#define DKLIB_APPLE_OSX

		#ifdef DKLIB_DYNAMIC		// dylib or Framework
			#define DKLIB_API	__attribute__((visibility ("default")))
			#warning Build DK for Mac OS X Dynamic Library.
		#else
			#define DKLIB_API
			#warning Build DK for Mac OS X Static Library.
		#endif
	#endif
#endif //if defined(__APPLE__) && defined(__MACH__)

////////////////////////////////////////////////////////////////////////////////
// LINUX, ANDROID
#ifdef __linux__
	#include <sys/endian.h>
	#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
		#if __BYTE_ORDER == __LITTLE_ENDIAN
			#define __LITTLE_ENDIAN__ 1
		#elif __BYTE_ORDER == __BIG_ENDIAN
			#define __BIG_ENDIAN__ 1
		#endif
	#endif

	#define DKLIB_LINUX
	// no default library on Linux. You should define static or dynamic.
	#if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
		#error "You should define DKLIB_STATIC or DKLIB_DYNAMIC"
	#endif

	#ifdef DKLIB_DYNAMIC		// so
		#define DKLIB_API	__attribute__((visibility ("default")))
		#warning Build DK for Linux Shared Library.
	#else
		#define DKLIB_API
		#warning Build DK for Linux Static Library.
	#endif
#endif

// Unsupported OS?
#if !defined(DKLIB_STATIC) && !defined(DKLIB_DYNAMIC)
#error "You should define DKLIB_STATIC or DKLIB_DYNAMIC"
#endif

// Macros for byte order conversion.
// byte order should be defined, It will be verified at run-time.
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	#if defined(_M_X64) || defined(_M_IX86) || defined(__i386__)
		#define __LITTLE_ENDIAN__	1
	#elif defined(__ARMEB__)
		#define __BIG_ENDIAN__ 1
	#else
		#error "System byte order not defined."
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// useful templates (C++ only)
#ifdef __cplusplus

namespace DKFoundation
{
	// Min, Max, Clamp
	template <typename T> inline T Max(T lhs, T rhs)			{return (lhs > rhs) ? lhs : rhs;}
	template <typename T> inline T Min(T lhs, T rhs)			{return (lhs < rhs) ? lhs : rhs;}
	template <typename T> inline T Clamp(T v, T a, T b)			{return Min<T>(Max<T>(v,a),b);}

	DKLIB_API void DKErrorRaiseException(const char*, const char*, unsigned int, const char*);
}

////////////////////////////////////////////////////////////////////////////////
// Macros for error, exception (DKError), C++ only.

#if			defined(_WIN32)
#define DKLIB_FUNCTION_NAME		__FUNCTION__
#elif		defined(__GNUC__)
#define DKLIB_FUNCTION_NAME		__PRETTY_FUNCTION__
#else
#define DKLIB_FUNCTION_NAME		__func__
#endif

#define DKERROR_THROW(desc)			DKFoundation::DKErrorRaiseException(DKLIB_FUNCTION_NAME, __FILE__, __LINE__, desc)
#define DKASSERT_DESC(expr, desc)	{if (!(expr)) DKERROR_THROW(desc);}
#define DKASSERT(expr)				{if (!(expr)) DKERROR_THROW("");}

#ifdef DKLIB_DEBUG_ENABLED
#define DKERROR_THROW_DEBUG(desc)		DKERROR_THROW(desc)
#define DKASSERT_DESC_DEBUG(expr, desc)	DKASSERT_DESC(expr, desc)
#define DKASSERT_DEBUG(expr)			DKASSERT(expr)
#else
#define DKERROR_THROW_DEBUG(desc)		(void)0
#define DKASSERT_DESC_DEBUG(expr, desc)	(void)0
#define DKASSERT_DEBUG(expr)			(void)0
#endif

#endif	// #ifdef __cplusplus

////////////////////////////////////////////////////////////////////////////////
// DK framework info
#ifdef __cplusplus
extern "C" {
#endif
	DKLIB_API const char* DKVersion(void);
	DKLIB_API const char* DKCopyright(void);
#ifdef __cplusplus
}
#endif
