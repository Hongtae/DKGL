﻿//
//  File: DKBlendState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKColor.h"

namespace DKFramework
{
	/// @brief blend-state for rendering.
	class DKGL_API DKBlendState
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

		static DKString FuncToString(const BlendFunc f);
		static DKString ModeToString(const BlendMode m);
		static BlendFunc StringToFunc(const DKString& str);
		static BlendMode StringToMode(const DKString& str);

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
