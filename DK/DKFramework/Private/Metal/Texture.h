//
//  File: Texture.h
//  Platform: OS X, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_METAL
#import <Metal/Metal.h>

#include "../../DKTexture.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			class Texture : public DKTexture
			{
			public:
				Texture(id<MTLTexture>);
				~Texture(void);

				uint32_t Width(void) override;
				uint32_t Height(void) override;
				uint32_t Depth(void) override;
				uint32_t MipmapCount(void) override;

				Type TextureType(void) override;
				DKPixelFormat PixelFormat(void) override;

			protected:
				id<MTLTexture> texture;
			};
		}
	}
}

#endif //#if DKGL_USE_METAL
