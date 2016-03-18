//
//  File: DKAudioDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKAudioDevice
// Audio Device (OpenAL) context object.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKAudioDevice : public DKFoundation::DKSharedInstance<DKAudioDevice>
	{
	public:
		~DKAudioDevice(void);

		bool IsBound(void) const;
		void Bind(void) const;
		void Unbind(void) const;

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
		friend class DKFoundation::DKObject<DKAudioDevice>;
		friend class DKFoundation::DKSharedInstance<DKAudioDevice>;
		DKAudioDevice(void);
		DKAudioDevice(const DKAudioDevice&);
		DKAudioDevice& operator = (const DKAudioDevice&);

		static DKFoundation::DKSpinLock		contextLock;
		DeviceList							deviceList;
		void *device;		// OpenAL device
		void *context;		// OpenAL context
	};
}
