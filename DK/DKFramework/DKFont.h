//
//  File: DKFont.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKGL_API DKFont
	{
	public:
		struct GlyphData
		{
			//DKObject<DKTexture2D>		texture;
			void* texture;
			DKPoint									position;
			DKSize									advance;
			DKRect									rect;
		};

		DKFont(void);
		~DKFont(void);

		// create object from file (path)
		static DKObject<DKFont> Create(const DKString& file);
		// create object from data pointer. (data will be copied inside)
		static DKObject<DKFont> Create(void* data, size_t size);
		// create object from DKData object. (data must not be modified by outside)
		static DKObject<DKFont> Create(DKData* data);
		// create object from stream, stream can be copied if necessary.
		static DKObject<DKFont> Create(DKStream* stream);

		const GlyphData* GlyphDataForChar(wchar_t c) const;

		// LineWidth: text pixel-width from baseline. not includes outline.
		float LineWidth(const DKString& str) const;
		// LineHeight: pixel-height from baseline. not includes outline.
		float LineHeight(void) const;

		float Baseline(void) const;	// baseline offset (in pixel unit)
		DKRect Bounds(const DKString& str) const; // text bounding box.
		DKPoint	KernAdvance(wchar_t left, wchar_t right) const; // calculate kern advance between characters.

		int				PointSize(void) const							{ return pointSize; }
		const DKPoint&	Resolution(void) const							{ return resolution; }
		float			Outline(void) const								{ return outline; }
		float			Embolden(void) const							{ return embolden; }
		bool			KerningEnabled(void) const						{ return kerningEnabled; }
		bool			ForceBitmap(void) const							{ return forceBitmap; }

		DKString FamilyName(void) const;
		DKString StyleName(void) const;

		// point, embolden is point-size, outline is pixel-size.
		bool SetStyle(int point, float embolden = 0, float outline = 0, DKPoint dpi = DKPoint(72,72), bool enableKerning = true, bool forceBitmap = false);
		bool IsValid(void) const;

		float Height(void) const;	// font pixel-height (includes outline)
		float Width(void) const;	// font pixel-width (includes outline)

		void ClearCache(void);		// clear glyph textures.

	private:
		float		outline;			// 0 for no-outline
		float		embolden;			// 0 for regular font
		int			pointSize;			// point size based DPI
		DKPoint		resolution;			// DPI resolution
		bool		kerningEnabled;		// kerning on/off
		bool		forceBitmap;		// force bitmap loads

		struct GlyphTextureAtlas
		{
			//DKObject<DKTexture2D>	texture;
			void* texture;
			unsigned int freeSpaceWidth;
		};

		typedef DKMap<wchar_t, GlyphData>		GlyphDataMap;
		typedef DKMap<wchar_t, unsigned int>	CharIndexMap;
		
		mutable GlyphDataMap				glyphMap;
		mutable CharIndexMap				charIndexMap;
		mutable DKArray<GlyphTextureAtlas> 	textures;
		mutable unsigned int				numGlyphsLoaded;

		void* ftFace;
		DKSpinLock lock;
		DKObject<DKData> fontData;

		//DKTexture2D* CacheGlyphTexture(int width, int height, void* data, DKRect& rect) const;
		void* CacheGlyphTexture(int width, int height, void* data, DKRect& rect) const { return NULL; }
	};
}
