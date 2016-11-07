//
//  File: DKGraphicsDevice.h
//  Platform: Linux, Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//


#include "../../Interface/DKGraphicsDeviceInterface.h"

namespace DKGL
{
	namespace Private
	{
		namespace Vulkan
		{
			class GraphicsDevice : public DKGraphicsDeviceInterface
			{
			public:
				~GraphicsDevice(void);
			};
		}
	}
}
