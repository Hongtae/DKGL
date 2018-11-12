//
//  File: GraphicsAPI.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#  ifndef DKGL_ENABLE_METAL
#    define DKGL_ENABLE_METAL 1
#  endif
#  ifndef DKGL_ENABLE_VULKAN
#    define DKGL_ENABLE_VULKAN 0
#  endif
#  ifndef DKGL_ENABLE_DIRECT3D12
#    define DKGL_ENABLE_DIRECT3D12 0
#  endif
#endif

#ifdef _WIN32
#  ifndef DKGL_ENABLE_VULKAN
#    define DKGL_ENABLE_VULKAN 1
#  endif
#  ifndef DKGL_ENABLE_METAL
#    define DKGL_ENABLE_METAL 0
#  endif
#  ifndef DKGL_ENABLE_DIRECT3D12
#    define DKGL_ENABLE_DIRECT3D12 0
#  endif
#endif

#ifdef __linux__
#  ifndef DKGL_ENABLE_VULKAN
#    define DKGL_ENABLE_VULKAN 1
#  endif
#  ifndef DKGL_ENABLE_METAL
#    define DKGL_ENABLE_METAL 0
#  endif
#  ifndef DKGL_ENABLE_DIRECT3D12
#    define DKGL_ENABLE_DIRECT3D12 0
#  endif
#endif


namespace DKFramework
{
	// useful key (DKPropertySet::SystemConfig)
	constexpr const char* graphicsDeviceListKey = "GraphicsDeviceList";
	constexpr const char* preferredGraphicsAPIKey = "GraphicsAPI";
	constexpr const char* preferredDeviceNameKey = "PreferredGraphicsDeviceName";

	// saved states key (DKPropertySet::DefaultSet)
	constexpr const char* savedSystemStates = "_SavedSystemStates"; // root
	constexpr const char* savedPipelineCacheDataKey = "PipelineCacheData";
}
