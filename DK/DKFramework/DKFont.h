//
//  File: DKFont.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKRect.h"

////////////////////////////////////////////////////////////////////////////////
// DKFont
// font object which contains glyph data.
// glyph data saved as OpenGL texture internally.
//
// Note:
//   If object created with font data as DKData object,
//   the data object must not be modified after object created.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKTexture2D;
	class DKLIB_API DKFont
	{
	public:
		struct GlyphData
		{
			DKFoundation::DKObject<DKTexture2D>		texture;
			DKPoint									position;
			DKSize									advance;
			DKRect									rect;
		};

		DKFont(void);
		~DKFont(void);

		// create object from file (path)
		static DKFoundation::DKObject<DKFont> Create(const DKFoundation::DKString& file);
		// create object from data pointer. (data will be copied inside)
		static DKFoundation::DKObject<DKFont> Create(void* data, size_t size);
		// create object from DKData object. (data must not be modified by outside)
		static DKFoundation::DKObject<DKFont> Create(DKFoundation::DKData* data);
		// create object from stream, stream can be copied if necessary.
		static DKFoundation::DKObject<DKFont> Create(DKFoundation::DKStream* stream);

		const GlyphData* GetGlyphData(wchar_t c) const;

		// LineWidth: text pixel-width from baseline. not includes outline.
		float LineWidth(const DKFoundation::DKString& str) const;
		// LineHeight: pixel-height from baseline. not includes outline.
		float LineHeight(void) const;

		float Baseline(void) const;	// baseline offset (in pixel unit)
		DKRect Bounds(const DKFoundation::DKString& str) const; // text bounding box.
		DKPoint	KernAdvance(wchar_t left, wchar_t right) const; // calculate kern advance between characters.

		int				PointSize(void) const							{ return pointSize; }
		const DKPoint&	Resolution(void) const							{ return resolution; }
		float			Outline(void) const								{ return outline; }
		float			Embolden(void) const							{ return embolden; }
		bool			KerningEnabled(void) const						{ return kerningEnabled; }
		bool			ForceBitmap(void) const							{ return forceBitmap; }

		DKFoundation::DKString FamilyName(void) const;
		DKFoundation::DKString StyleName(void) const;

		// point, embolden is point-size, outline is pixel-size.
		bool SetStyle(int point, float embolden = 0, float outline = 0, DKPoint dpi = DKPoint(72,72), bool enableKerning = true, bool forceBitmap = false);
		bool IsValid(void) const;

		float Height(void) const;  // font pixel-height (includes outline)
		float Width(void) const;   // font pixel-width (includes outline)

	private:
		float		outline;			// 0 for no-outline
		float		embolden;			// 0 for regular font
		int			pointSize;			// point size based DPI
		DKPoint		resolution;			// DPI resolution
		bool		kerningEnabled;		// kerning on/off
		bool		forceBitmap;		// force bitmap loads

		struct SharedTextures
		{
			DKFoundation::DKObject<DKTexture2D>	texture;
			unsigned int freeSpaceWidth;
		};
		typedef DKFoundation::DKMap<wchar_t, GlyphData>		GlyphDataMap;
		typedef DKFoundation::DKMap<wchar_t, unsigned int>	CharIndexMap;
		
		mutable GlyphDataMap								glyphMap;
		mutable CharIndexMap								charIndexMap;
		mutable DKFoundation::DKArray<SharedTextures>		textures;
		mutable unsigned int								numGlyphsLoaded;

		void* ftFace;
		DKFoundation::DKSpinLock lock;
		DKFoundation::DKObject<DKFoundation::DKData> fontData;
		DKTexture2D* GetGlyphTexture(int width, int height, void* data, DKRect& rect) const;
	};
}
