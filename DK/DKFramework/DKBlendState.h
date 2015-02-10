//
//  File: DKBlendState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKColor.h"

////////////////////////////////////////////////////////////////////////////////
// DKBlendState
// class for OpenGL blend functions.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKBlendState
	{
	public:
		enum BlendMode : unsigned char
		{
			BlendModeZero = 0,
			BlendModeOne,
			BlendModeSrcColor,
			BlendModeOneMinusSrcColor,
			BlendModeDstColor,
			BlendModeOneMinusDstColor,
			BlendModeSrcAlpha,
			BlendModeOneMinusSrcAlpha,
			BlendModeDstAlpha,
			BlendModeOneMinusDstAlpha,
			BlendModeConstantColor,
			BlendModeOneMinusConstColor,
			BlendModeConstantAlpha,
			BlendModeOneMinusConstAlpha,
			BlendModeSrcAlphaSaturate,
		};
		enum BlendFunc : unsigned char
		{
			BlendFuncAdd = 0,
			BlendFuncSubtract,
			BlendFuncReverseSubtract,
			BlendFuncMin,
			BlendFuncMax,
		};

		static DKFoundation::DKString FuncToString(const BlendFunc f);
		static DKFoundation::DKString ModeToString(const BlendMode m);
		static BlendFunc StringToFunc(const DKFoundation::DKString& str);
		static BlendMode StringToMode(const DKFoundation::DKString& str);

		DKBlendState(BlendMode srcRGB,
					 BlendMode srcAlpha,
					 BlendMode dstRGB,
					 BlendMode dstAlpha,
					 BlendFunc funcRGB,
					 BlendFunc funcAlpha,
					 bool writeR = true,
					 bool writeG = true,
					 bool writeB = true,
					 bool writeA = true,
					 const DKColor& color = DKColor(0,0,0,0));

		DKBlendState(BlendMode src,
					 BlendMode dst,
					 BlendFunc func,
					 bool writeR = true,
					 bool writeG = true,
					 bool writeB = true,
					 bool writeA = true,
					 const DKColor& color = DKColor(0,0,0,0));
		DKBlendState(void);

		~DKBlendState(void);

		void Bind(void) const;

		bool			colorWriteR:1;
		bool			colorWriteG:1;
		bool			colorWriteB:1;
		bool			colorWriteA:1;
		BlendMode		srcBlendRGB;
		BlendMode		srcBlendAlpha;
		BlendMode		dstBlendRGB;
		BlendMode		dstBlendAlpha;
		BlendFunc		blendFuncRGB;
		BlendFunc		blendFuncAlpha;
		DKColor::RGBA32	constantColor;

		// preset
		static const DKBlendState	defaultOpaque;
		static const DKBlendState	defaultAlpha;
		static const DKBlendState	defaultMultiply;
		static const DKBlendState	defaultScreen;
		static const DKBlendState	defaultDarken;
		static const DKBlendState	defaultLighten;
		static const DKBlendState	defaultLinearBurn;
		static const DKBlendState	defaultLinearDodge;
	};
}
