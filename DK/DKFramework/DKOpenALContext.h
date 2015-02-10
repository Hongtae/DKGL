//
//  File: DKOpenALContext.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKOpenALContext
// OpenAL context object.
//
// Note:
//    On iOS:
//      DKOpenALContext::IsActivated returns false, if audio-session of iOS has
//      been deactivated.
//      If you don't use DKApplication, You have to call
//      DKOpenALContext::Deactivate() when audio-session interruption begin,
//      and call DKOpenALContext::Activate() when interruption ended.
//      If you are using DKApplication class, never call
//      DKOpenALContext::Deactivate(), DKOpenALContext::Activate().
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKOpenALContext : public DKFoundation::DKSharedInstance<DKOpenALContext>
	{
	public:
		~DKOpenALContext(void);

		bool IsBound(void) const;
		void Bind(void) const;
		void Unbind(void) const;

		bool IsActivated(void) const; // check system can play audio

		// device activate, deactivate, called by DKApplication automatically.
		static void Activate(void);   // Never call if you're using DKApplication.
		static void Deactivate(void); // Never call if you're using DKApplication.

		struct DeviceList
		{
			struct Device
			{
				DKFoundation::DKString name;
				int majorVersion;
				int minorVersion;
			};
			DKFoundation::DKString			defaultDeivce;
			DKFoundation::DKArray<Device>	list;
		};

		//static bool debugMode;
	private:
		friend class DKFoundation::DKObject<DKOpenALContext>;
		friend class DKFoundation::DKSharedInstance<DKOpenALContext>;
		DKOpenALContext(void);
		DKOpenALContext(const DKOpenALContext&);
		DKOpenALContext& operator = (const DKOpenALContext&);

		static DKFoundation::DKSpinLock		contextLock;
		DeviceList							deviceList;
		void *device;		// OpenAL device
		void *context;		// OpenAL context
	};
}
