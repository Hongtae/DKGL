//
//  File: Texture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_DIRECT3D12
#include "d3d12_headers.h"

#include "../../DKTexture.h"
#include "../../DKGraphicsDevice.h"
#include "TextureBaseT.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class Texture : public TextureBaseT<DKTexture>
			{
			public:
				Texture(DKGraphicsDevice*, ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE);
				~Texture(void);

			private:
				DKObject<DKGraphicsDevice> device;
				D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
			};
		}
	}
}

#endif //#if DKGL_ENABLE_DIRECT3D12
