//
//  File: DKAudioDevice.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// Audio device context.
	/// Currently, OpenAL backend implemented.
	class DKGL_API DKAudioDevice : public DKSharedInstance<DKAudioDevice>
	{
	public:
		~DKAudioDevice();

		bool IsBound() const;
		void Bind() const;
		void Unbind() const;

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
		DKAudioDevice();
		DKAudioDevice(const DKAudioDevice&);
		DKAudioDevice& operator = (const DKAudioDevice&) = delete;

		DeviceList deviceList;
		void *device;		// OpenAL device
		void *context;		// OpenAL context
	};
}
