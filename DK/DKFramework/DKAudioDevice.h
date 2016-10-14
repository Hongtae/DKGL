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

namespace DKGL
{
	class DKGL_API DKAudioDevice : public DKSharedInstance<DKAudioDevice>
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
				DKString name;
				int majorVersion;
				int minorVersion;
			};
			DKString		defaultDeivce;
			DKArray<Device>	list;
		};

		//static bool debugMode;
	private:
		friend class DKObject<DKAudioDevice>;
		friend class DKSharedInstance<DKAudioDevice>;
		DKAudioDevice(void);
		DKAudioDevice(const DKAudioDevice&);
		DKAudioDevice& operator = (const DKAudioDevice&);

		DeviceList deviceList;
		void *device;		// OpenAL device
		void *context;		// OpenAL context
	};
}
