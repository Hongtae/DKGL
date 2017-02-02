//
//  File: RenderTarget.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_USE_DIRECT3D
#include "d3d12_headers.h"

#include "../../DKRenderTarget.h"
#include "TextureBaseT.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Direct3D
		{
			class GraphicsDevice;
			class RenderTarget : public TextureBaseT<DKRenderTarget>
			{
			public:
				RenderTarget(GraphicsDevice*, ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE srv, D3D12_CPU_DESCRIPTOR_HANDLE rtv);
				~RenderTarget(void);

				GraphicsDevice* device;
				D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
			};
		}
	}
}

#endif //#if DKGL_USE_DIRECT3D
