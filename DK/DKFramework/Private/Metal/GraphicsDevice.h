//
//  File: GraphicsDevice.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../Interface/DKGraphicsDeviceInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			DKGraphicsDeviceInterface* CreateInterface(void);

			class GraphicsDevice : public DKGraphicsDeviceInterface
			{
			public:
				GraphicsDevice(void);
				~GraphicsDevice(void);

				DKObject<DKCommandQueue> CreateCommandQueue(DKGraphicsDevice*) override;

			private:
				id<MTLDevice> device;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
