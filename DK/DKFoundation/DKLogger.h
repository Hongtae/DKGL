//
//  File: DKLogger.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKString.h"

namespace DKFoundation
{
	enum class DKLogCategory;

	/// @brief a logger class.
	/// you can sublcass DKLogger to handle log text.
	class DKGL_API DKLogger
	{
	public:
		using Category = DKLogCategory;

		DKLogger(void);
		virtual ~DKLogger(void);

		/// Should override this function, Never call Unbind() in this function!
		virtual void Log(Category, const DKString&) = 0;

		void Bind(void);	/// bind to system, share ownership with system.
		void Unbind(void);
		bool IsBound(void) const;

		static size_t Broadcast(Category, const DKString&);

		static DKObject<DKLogger> CreateSimpleLogger(void(*)(Category, const DKString&));
	protected:
		virtual void OnBind(void) {}
		virtual void OnUnbind(void) {}
	};
}
