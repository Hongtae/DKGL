//
//  File: DKLog.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

namespace DKFoundation
{
	enum class DKLogCategory { Verbose = 'V', Info = 'I', Debug = 'D', Warning = 'W', Error = 'E' };

	DKGL_API void DKLog(DKLogCategory, const DKString& str);
	DKGL_API void DKLog(DKLogCategory, const char* fmt, ...);

	template <DKLogCategory category = DKLogCategory::Verbose>
	inline void DKLog(const DKString& str)
	{
		DKLog(category, str);
	}
	template <DKLogCategory category = DKLogCategory::Verbose, typename... Args> 
	inline void DKLog(const char* fmt, Args&&... args)
	{
		DKLog(category, fmt, std::forward<Args>(args)...);
	}

	template <typename Str, typename... Args> inline void DKLogV(Str&& s, Args&&... args)
	{
		DKLog<DKLogCategory::Verbose>(std::forward<Str>(s), std::forward<Args>(args)...);
	}
	template <typename Str, typename... Args> inline void DKLogI(Str&& s, Args&&... args)
	{
		DKLog<DKLogCategory::Info>(std::forward<Str>(s), std::forward<Args>(args)...);
	}
	template <typename Str, typename... Args> inline void DKLogD(Str&& s, Args&&... args)
	{
		DKLog<DKLogCategory::Debug>(std::forward<Str>(s), std::forward<Args>(args)...);
	}
	template <typename Str, typename... Args> inline void DKLogW(Str&& s, Args&&... args)
	{
		DKLog<DKLogCategory::Warning>(std::forward<Str>(s), std::forward<Args>(args)...);
	}
	template <typename Str, typename... Args> inline void DKLogE(Str&& s, Args&&... args)
	{
		DKLog<DKLogCategory::Error>(std::forward<Str>(s), std::forward<Args>(args)...);
	}
}

#ifdef DKGL_DEBUG_ENABLED
#define DKLOG_DEBUG(...)	DKLog(__VA_ARGS__)
#else
#define DKLOG_DEBUG(...)	()0
#endif
