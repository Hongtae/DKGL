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
		const char* defaultAPI =
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

		std::initializer_list<APISet> apis = {
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

		if (true)
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

		DKStringU8 selectAPI = "";

		// get user preferred API name.
		if (true)
		{
			const char* key = "GraphicsAPI";
			DKPropertySet& config = DKPropertySet::SystemConfig();
			if (config.HasValue(key))
			{
				const DKVariant& var = config.Value(key);
				if (var.ValueType() == DKVariant::TypeString)
				{
					selectAPI = DKStringU8(var.String());
				}
			}
		}

		auto getInterface = [](const APISet& as) -> DKGraphicsDeviceInterface*
		{
			DKLog("Graphics API '%s' selected.", as.name);
			return as.fn();
		};

		// find user preferred API first.
		for (const APISet& as : apis)
		{
			if (selectAPI.CompareNoCase(as.name) == 0)
				return getInterface(as);
		}
		// find default API.
		selectAPI = defaultAPI;
		for (const APISet& as : apis)
		{
			if (selectAPI.CompareNoCase(as.name) == 0)
				return getInterface(as);
		}
		// get first available API in list.
		for (const APISet& as : apis)
		{
			return getInterface(as);
		}

		return NULL;
	}
}
