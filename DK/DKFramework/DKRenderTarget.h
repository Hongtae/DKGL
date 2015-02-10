//
//  File: DKRenderTarget.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKTexture2D.h"

////////////////////////////////////////////////////////////////////////////////
// DKRenderTarget
// render target class. manage OpenGL FBO object, which made for multi threads.
// (Note: OpenGL's FOB is not shared between threads)
//
// a texture object can be used as color buffer,
// a depth texture or render buffer can be used as depth buffer.
// the render buffer can not be used as color buffer in this class.
//
// Note:
//     In spite of this class provides FBO for multi-threads,
//     some functions are not thread safe. You need to set up this class before
//     sharing with other threads.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKRenderTarget
	{
	public:
		enum DepthFormat
		{
			DepthFormatNone = 0,
			DepthFormat16,
			DepthFormat24,
			DepthFormat32,
		};

		struct TargetQuery
		{
			unsigned int width;
			unsigned int height;
			DepthFormat depth;
			unsigned int fbo;
		};
		typedef DKFoundation::DKInvocation<TargetQuery> ProxyQuery;

		static DKFoundation::DKObject<DKRenderTarget> Create(int width, int height, DepthFormat depth);
		static DKFoundation::DKObject<DKRenderTarget> Create(ProxyQuery* proxy);

		DKRenderTarget(void);
		virtual ~DKRenderTarget(void);

		bool SetColorTextures(DKTexture2D** tex, size_t num);
		DKTexture2D* ColorTexture(int index);
		const DKTexture2D* ColorTexture(int index) const;
		size_t NumberOfColorTextures(void) const;
		
		static size_t MaxColorTextures(void);

		bool SetDepthTexture(DKTexture2D* tex);
		bool SetDepthBuffer(int width, int height, DepthFormat df);
		DKTexture2D* DepthTexture(void);
		const DKTexture2D* DepthTexture(void) const;
		DepthFormat DepthBufferFormat(void) const;

		virtual bool Bind(void) const;
		virtual bool IsValid(void) const;
		bool Validate(void);
		bool IsProxy(void) const;

		bool CopyColorTexture(int target, DKTexture2D* tex) const;
		bool CopyDepthTexture(DKTexture2D* tex) const;
		
		DKSize Resolution(void) const;

	private:
		DKSize depthResolution;
		DepthFormat	depthFormat;

		void UpdateResolution(void);

		DKFoundation::DKArray<DKFoundation::DKObject<DKTexture2D>>	colorTextures;
		DKFoundation::DKObject<DKTexture2D>							depthTexture;  // optional
		unsigned int												depthBuffer;

		DKFoundation::DKObject<ProxyQuery>	proxyQuery;		
	};
}
