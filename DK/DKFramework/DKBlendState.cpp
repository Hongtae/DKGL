//
//  File: DKBlendState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/Inc_OpenGL.h"
#include "DKBlendState.h"
#include "DKOpenGLContext.h"

namespace DKGL
{
	namespace Private
	{
		namespace
		{
			inline GLenum GetGLValue(DKBlendState::BlendFunc f)
			{
				switch (f)
				{
				case DKBlendState::BlendFuncAdd:					return GL_FUNC_ADD;
				case DKBlendState::BlendFuncSubtract:				return GL_FUNC_SUBTRACT;
				case DKBlendState::BlendFuncReverseSubtract:		return GL_FUNC_REVERSE_SUBTRACT;
				case DKBlendState::BlendFuncMin:					return GL_MIN;
				case DKBlendState::BlendFuncMax:					return GL_MAX;
				}
				DKLog("Warning: BlendFunc (%x) invalid or not supported.\n", f);
				return GL_FUNC_ADD;
			}
			inline GLenum GetGLValue(DKBlendState::BlendMode m)
			{
				switch (m)
				{
				case DKBlendState::BlendModeZero:					return GL_ZERO;
				case DKBlendState::BlendModeOne:					return GL_ONE;
				case DKBlendState::BlendModeSrcColor:				return GL_SRC_COLOR;
				case DKBlendState::BlendModeOneMinusSrcColor:		return GL_ONE_MINUS_SRC_COLOR;
				case DKBlendState::BlendModeDstColor:				return GL_DST_COLOR;
				case DKBlendState::BlendModeOneMinusDstColor:		return GL_ONE_MINUS_DST_COLOR;
				case DKBlendState::BlendModeSrcAlpha:				return GL_SRC_ALPHA;
				case DKBlendState::BlendModeOneMinusSrcAlpha:		return GL_ONE_MINUS_SRC_ALPHA;
				case DKBlendState::BlendModeDstAlpha:				return GL_DST_ALPHA;
				case DKBlendState::BlendModeOneMinusDstAlpha:		return GL_ONE_MINUS_DST_ALPHA;
				case DKBlendState::BlendModeConstantColor:			return GL_CONSTANT_COLOR;
				case DKBlendState::BlendModeOneMinusConstColor:	return GL_ONE_MINUS_CONSTANT_COLOR;
				case DKBlendState::BlendModeConstantAlpha:			return GL_CONSTANT_ALPHA;
				case DKBlendState::BlendModeOneMinusConstAlpha:	return GL_ONE_MINUS_CONSTANT_ALPHA;
				case DKBlendState::BlendModeSrcAlphaSaturate:		return GL_SRC_ALPHA_SATURATE;
				}
				DKLog("Warning: BlendMode (%x) invalid or not supported.\n", m);
				return GL_ZERO;
			}
		}
	}
}

using namespace DKGL;
using namespace DKGL;


const DKBlendState	DKBlendState::defaultOpaque(BlendModeOne, BlendModeZero, BlendFuncAdd);
const DKBlendState	DKBlendState::defaultAlpha(BlendModeSrcAlpha, BlendModeOne, BlendModeOneMinusSrcAlpha, BlendModeOne, BlendFuncAdd, BlendFuncAdd);
const DKBlendState	DKBlendState::defaultMultiply(BlendModeZero, BlendModeSrcColor, BlendFuncAdd);
const DKBlendState	DKBlendState::defaultScreen(BlendModeOneMinusDstColor, BlendModeOne, BlendFuncAdd);
const DKBlendState	DKBlendState::defaultDarken(BlendModeOne, BlendModeOne, BlendFuncMin);
const DKBlendState	DKBlendState::defaultLighten(BlendModeOne, BlendModeOne, BlendFuncMax);
const DKBlendState	DKBlendState::defaultLinearBurn(BlendModeOne, BlendModeOneMinusDstColor, BlendFuncSubtract);
const DKBlendState	DKBlendState::defaultLinearDodge(BlendModeOne, BlendModeOne, BlendFuncAdd);


DKBlendState::DKBlendState(BlendMode srcRGB,
						   BlendMode srcAlpha,
						   BlendMode dstRGB,
						   BlendMode dstAlpha,
						   BlendFunc funcRGB,
						   BlendFunc funcAlpha,
						   bool writeR,
						   bool writeG,
						   bool writeB,
						   bool writeA,
						   const DKColor& color)
: srcBlendRGB(srcRGB)
, srcBlendAlpha(srcAlpha)
, dstBlendRGB(dstRGB)
, dstBlendAlpha(dstAlpha)
, blendFuncRGB(funcRGB)
, blendFuncAlpha(funcAlpha)
, colorWriteR(writeR)
, colorWriteG(writeG)
, colorWriteB(writeB)
, colorWriteA(writeA)
, constantColor(color.RGBA32Value())
{
}

DKBlendState::DKBlendState(BlendMode src,
						   BlendMode dst,
						   BlendFunc func,
						   bool writeR,
						   bool writeG,
						   bool writeB,
						   bool writeA,
						   const DKColor& color)
: srcBlendRGB(src)
, srcBlendAlpha(src)
, dstBlendRGB(dst)
, dstBlendAlpha(dst)
, blendFuncRGB(func)
, blendFuncAlpha(func)
, colorWriteR(writeR)
, colorWriteG(writeG)
, colorWriteB(writeB)
, colorWriteA(writeA)
, constantColor(color.RGBA32Value())
{
}

DKBlendState::DKBlendState(void)
: srcBlendRGB(BlendModeOne)
, srcBlendAlpha(BlendModeOne)
, dstBlendRGB(BlendModeZero)
, dstBlendAlpha(BlendModeZero)
, blendFuncRGB(BlendFuncAdd)
, blendFuncAlpha(BlendFuncAdd)
, colorWriteR(true)
, colorWriteG(true)
, colorWriteB(true)
, colorWriteA(true)
, constantColor({0,0,0,0})
{
}

DKBlendState::~DKBlendState(void)
{
}

void DKBlendState::Bind(void) const
{
	DKRenderState&	state = DKOpenGLContext::RenderState();
	state.ColorMask(colorWriteR, colorWriteG, colorWriteB, colorWriteA);
	DKColor c = this->constantColor;
	state.BlendColor(c.r, c.g, c.b, c.a);
	state.BlendEquation(Private::GetGLValue(blendFuncRGB), Private::GetGLValue(blendFuncAlpha));
	state.BlendFunc(Private::GetGLValue(srcBlendRGB), Private::GetGLValue(srcBlendAlpha), Private::GetGLValue(dstBlendRGB), Private::GetGLValue(dstBlendAlpha));

	if (blendFuncRGB == blendFuncAlpha && blendFuncRGB == BlendFuncAdd &&
		srcBlendRGB == srcBlendAlpha && srcBlendRGB == BlendModeOne &&
		dstBlendRGB == dstBlendAlpha && dstBlendRGB == BlendModeZero)
		state.Disable(DKRenderState::GLStateBlend);
	else
		state.Enable(DKRenderState::GLStateBlend);
}

DKString DKBlendState::FuncToString(const BlendFunc f)
{
	switch (f)
	{
	case BlendFuncAdd:					return L"Add";
	case BlendFuncSubtract:				return L"Subtract";
	case BlendFuncReverseSubtract:		return L"ReverseSubtract";
	case BlendFuncMin:					return L"Min";
	case BlendFuncMax:					return L"Max";
	}
	return L"Add";		// default
}

DKString DKBlendState::ModeToString(const BlendMode m)
{
	switch (m)
	{
	case BlendModeZero:					return L"Zero";
	case BlendModeOne:					return L"One";
	case BlendModeSrcColor:				return L"SrcColor";
	case BlendModeOneMinusSrcColor:		return L"OneMinusSrcColor";
	case BlendModeDstColor:				return L"DstColor";
	case BlendModeOneMinusDstColor:		return L"OneMinusDstColor";
	case BlendModeSrcAlpha:				return L"SrcAlpha";
	case BlendModeOneMinusSrcAlpha:		return L"OneMinusSrcAlpha";
	case BlendModeDstAlpha:				return L"DstAlpha";
	case BlendModeOneMinusDstAlpha:		return L"OneMinusDstAlpha";
	case BlendModeConstantColor:		return L"ConstantColor";
	case BlendModeOneMinusConstColor:	return L"OneMinusConstColor";
	case BlendModeConstantAlpha:		return L"ConstantAlpha";
	case BlendModeOneMinusConstAlpha:	return L"OneMinusConstAlpha";
	case BlendModeSrcAlphaSaturate:		return L"SrcAlphaSaturate";
	}
	return L"Zero";		// default
}

DKBlendState::BlendFunc DKBlendState::StringToFunc(const DKString& str)
{
	if		(!str.CompareNoCase(FuncToString(BlendFuncAdd)))						return BlendFuncAdd;
	else if (!str.CompareNoCase(FuncToString(BlendFuncSubtract)))					return BlendFuncSubtract;
	else if (!str.CompareNoCase(FuncToString(BlendFuncReverseSubtract)))			return BlendFuncReverseSubtract;
	else if (!str.CompareNoCase(FuncToString(BlendFuncMin)))						return BlendFuncMin;
	else if (!str.CompareNoCase(FuncToString(BlendFuncMax)))						return BlendFuncMax;
	return BlendFuncAdd;
}

DKBlendState::BlendMode DKBlendState::StringToMode(const DKString& str)
{
	if		(!str.CompareNoCase(ModeToString(BlendModeZero)))						return BlendModeZero;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOne)))						return BlendModeOne;
	else if	(!str.CompareNoCase(ModeToString(BlendModeSrcColor)))					return BlendModeSrcColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusSrcColor)))			return BlendModeOneMinusSrcColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeDstColor)))					return BlendModeDstColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusDstColor)))			return BlendModeOneMinusDstColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeSrcAlpha)))					return BlendModeSrcAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusSrcAlpha)))			return BlendModeOneMinusSrcAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeDstAlpha)))					return BlendModeDstAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusDstAlpha)))			return BlendModeOneMinusDstAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeConstantColor)))				return BlendModeConstantColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusConstColor)))			return BlendModeOneMinusConstColor;
	else if	(!str.CompareNoCase(ModeToString(BlendModeConstantAlpha)))				return BlendModeConstantAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeOneMinusConstAlpha)))			return BlendModeOneMinusConstAlpha;
	else if	(!str.CompareNoCase(ModeToString(BlendModeSrcAlphaSaturate)))			return BlendModeSrcAlphaSaturate;
	return BlendModeZero;
}

