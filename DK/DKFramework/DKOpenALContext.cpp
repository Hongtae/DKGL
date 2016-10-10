//
//  File: DKOpenALContext.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenAL.h"
#include "DKOpenALContext.h"

namespace DKGL
{
	namespace Private
	{
		ALCcontext* bindContextWhenActivated = NULL;
		bool audioSessionActivated = true;
	}
}

using namespace DKGL;
using namespace DKGL;

DKSpinLock DKOpenALContext::contextLock;

DKOpenALContext::DKOpenALContext(void)
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

	context = (void*)alcCreateContext((ALCdevice*)device, NULL);

	if (context == NULL)	// failed.
	{
		DKERROR_THROW_DEBUG("alcCreateContext failed.");
	}
	DKLog("DKOpenALContext:0x%x Created.\n", this);
}

DKOpenALContext::~DKOpenALContext(void)
{
	DKCriticalSection<DKSpinLock> section(contextLock);

	if (alcGetCurrentContext() == (ALCcontext*)context)
		alcMakeContextCurrent(NULL);
	if (context)
		alcDestroyContext((ALCcontext*)context);
	if (device)
		alcCloseDevice((ALCdevice*)device);

	context = NULL;
	device = NULL;
	DKLog("DKOpenALContext:0x%x Destroyed.\n", this);
}

bool DKOpenALContext::IsBound(void) const
{
	return alcGetCurrentContext() == (ALCcontext*)context;
}

void DKOpenALContext::Bind(void) const
{
	if (Private::audioSessionActivated)
		alcMakeContextCurrent((ALCcontext*)context);
	else
		Private::bindContextWhenActivated = (ALCcontext*)context;		
}

void DKOpenALContext::Unbind(void) const
{
}

bool DKOpenALContext::IsActivated(void) const
{
	return Private::audioSessionActivated;
}

void DKOpenALContext::Activate(void)
{
	DKCriticalSection<DKSpinLock> section(contextLock);
	
	ALCcontext* current = alcGetCurrentContext();
	if (current == NULL && Private::audioSessionActivated == false)
	{
		alcMakeContextCurrent(Private::bindContextWhenActivated);
		
		Private::audioSessionActivated = true;
		Private::bindContextWhenActivated = NULL;
		
		DKLog("DKOpenALContext activated.\n");
	}
}

void DKOpenALContext::Deactivate(void)
{
	DKCriticalSection<DKSpinLock> section(contextLock);
	
	ALCcontext* current = alcGetCurrentContext();
	if (current || Private::audioSessionActivated)
	{
		Private::audioSessionActivated = false;
		Private::bindContextWhenActivated = current;
		alcMakeContextCurrent(NULL);
	}
	DKLog("DKOpenALContext deactivated.\n");
}
