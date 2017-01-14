//
//  File: GraphicsAPI.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include <initializer_list>
#include "../DKPropertySet.h"
#include "../Interface/DKGraphicsDeviceInterface.h"
#include "GraphicsAPI.h"

namespace DKFramework
{
	namespace Private
	{
#if DKGL_USE_METAL
		namespace Metal
		{
			DKGraphicsDeviceInterface* CreateInterface(void);
		}
#endif
#if DKGL_USE_VULKAN
		namespace Vulkan
		{
			DKGraphicsDeviceInterface* CreateInterface(void);
		}
#endif
#if DKGL_USE_DIRECT3D
		namespace Direct3D
		{
			DKGraphicsDeviceInterface* CreateInterface(void);
		}
#endif
#if DKGL_USE_OPENGL
		namespace OpenGL
		{
			DKGraphicsDeviceInterface* CreateInterface(void);
		}
#endif
	}

	DKGraphicsDeviceInterface* DKGraphicsDeviceInterface::CreateInterface(void)
	{
		DKString defaultAPI =
#if DKGL_USE_METAL
		"Metal";
#elif DKGL_USE_DIRECT3D
		"Direct3D";
#elif DKGL_USE_VULKAN
		"Vulkan";
#elif DKGL_USE_OPENGL
		"OpenGL";
#else
		"";
#endif
		struct APISet
		{
			const char* name;
			DKGraphicsDeviceInterface* (*fn)(void);
		};

		DKArray<APISet> apis = {
#if DKGL_USE_METAL
			{ "Metal", Private::Metal::CreateInterface },
#endif
#if DKGL_USE_DIRECT3D
			{ "Direct3D", Private::Direct3D::CreateInterface },
#endif
#if DKGL_USE_VULKAN
			{ "Vulkan", Private::Vulkan::CreateInterface },
#endif
#if DKGL_USE_OPENGL
			{ "OpenGL", Private::OpenGL::CreateInterface },
#endif
		};

		for (size_t i = 0; i < apis.Count(); ++i)
		{
			if (defaultAPI.CompareNoCase(apis.Value(i).name) == 0)
			{
				if (i > 0)
				{
					APISet api = apis.Value(i);
					apis.Remove(i);
					apis.Insert(api, 0);
				}
				break;
			}
		}

		// get user preferred API name.
		bool tryPreferredApiFirst = true;
		if (tryPreferredApiFirst)
		{
			const char* key = "GraphicsAPI";
			DKPropertySet& config = DKPropertySet::SystemConfig();
			if (config.HasValue(key))
			{
				const DKVariant& var = config.Value(key);
				if (var.ValueType() == DKVariant::TypeString)
				{
					DKString selectAPI = DKStringU8(var.String());

					for (size_t i = 0; i < apis.Count(); ++i)
					{
						if (selectAPI.CompareNoCase(apis.Value(i).name) == 0)
						{
							if (i > 0)
							{
								APISet api = apis.Value(i);
								apis.Remove(i);
								apis.Insert(api, 0);
							}
							break;
						}
					}
				}
			}
		}

		bool printApiSet = true;
		if (printApiSet)
		{
			int index = 0;
			for (const APISet& as : apis)
			{
				DKLog(" GraphicsAPI[%d]: %s", index, as.name);
				index++;
			}
			if (index == 0)
				DKLog(" No Graphics API available.");
		}

		for (const APISet& as : apis)
		{
			try {
				DKGraphicsDeviceInterface* device = as.fn();
				if (device)
				{
					DKLog("Graphics API \"%s\" selected.", as.name);
					return device;
				}
				else
				{
					DKLog("Graphics API \"%s\" not supported.", as.name);
				}
			}
			catch (DKError& e)
			{
				DKLog("Graphics API \"%s\" Failed: %ls", as.name, (const wchar_t*)e.Description());
			}
			catch (std::exception& e)
			{
				DKLog("Graphics API \"%s\" Failed: %s", as.name, e.what());
			}
		}

		DKLog("ERROR: No Graphics device found.");
		return NULL;
	}
}
