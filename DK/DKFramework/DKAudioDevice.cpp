//
//  File: DKAudioDevice.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/OpenAL.h"
#include "DKAudioDevice.h"

using namespace DKFramework;

DKAudioDevice::DKAudioDevice()
	: device(NULL)
	, context(NULL)
{	
	// get audio device descriptions.
	if (deviceList.list.IsEmpty() && alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE) 
	{
		// Enumeration Extension Found 

		// Pass in NULL device handle to get list of devices 
		const char* devices = (char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER); 
		// devices contains the device names, separated by NULL  
		// and terminated by two consecutive NULLs. 
		deviceList.defaultDeivce = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER); 
		// defaultDeviceName contains the name of the default device 

		while ( *devices != 0 )
		{
			ALCdevice *device = alcOpenDevice(devices);
			if (device)
			{
				ALCcontext *context = alcCreateContext(device, NULL);
				if (context)
				{
					DeviceList::Device	dev;

					alcMakeContextCurrent(context);
					dev.name = alcGetString(device, ALC_DEVICE_SPECIFIER);
					dev.majorVersion = 0;
					dev.minorVersion = 0;
					alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &dev.majorVersion);
					alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &dev.minorVersion);

					alcMakeContextCurrent(NULL);
					alcDestroyContext(context);

					deviceList.list.Add(dev);
				}
				alcCloseDevice(device);
			}
			devices += strlen(devices) + 1;
		}
	}
	else
	{
		DKLog("[DKAVMediaController] ALC_ENUMERATION_EXT not valid\n");
	}


	DKLog("OpenAL Devices: %d, Default Device: \"%ls\"\n", deviceList.list.Count(), (const wchar_t*)deviceList.defaultDeivce);
	for (int i = 0; i < deviceList.list.Count(); i++)
	{
		DKLog("OpenAL Device[%d]: \"%ls\", Version: %d.%d\n", i,
			(const wchar_t*)deviceList.list.Value(i).name,
			deviceList.list.Value(i).majorVersion,
			deviceList.list.Value(i).minorVersion);
	}

	// cleate context
	//device = (void*)alcOpenDevice("Generic Software");
	device = (void*)alcOpenDevice(NULL);
    if (device == nullptr)
    {
        DKLogE("alcOpenDevice failed. - NO SOUND DEVICE!");
    }
    else
    {
        context = (void*)alcCreateContext((ALCdevice*)device, NULL);

        if (context == NULL)	// failed.
        {
            //DKERROR_THROW_DEBUG("alcCreateContext failed.");
            DKLogE("alcCreateContext failed.");
        }
    }
	DKLog("DKAudioDevice:0x%x Created. (Device: 0x%x)\n", this, device);
}

DKAudioDevice::~DKAudioDevice()
{
	if (alcGetCurrentContext() == (ALCcontext*)context)
		alcMakeContextCurrent(NULL);
	if (context)
		alcDestroyContext((ALCcontext*)context);
	if (device)
		alcCloseDevice((ALCdevice*)device);

	context = NULL;
	device = NULL;
	DKLog("DKAudioDevice:0x%x Destroyed.\n", this);
}

bool DKAudioDevice::IsBound() const
{
	return alcGetCurrentContext() == (ALCcontext*)context;
}

void DKAudioDevice::Bind() const
{
	alcMakeContextCurrent((ALCcontext*)context);
}

void DKAudioDevice::Unbind() const
{
		alcMakeContextCurrent(NULL);
}
