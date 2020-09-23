//
//  File: DKFont.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKTexture.h"

namespace DKFramework
{
	/// @brief
	/// font object which contains glyph data.
	/// @details
	/// glyph data saved as GPU texture internally.
	///
	/// @note
	///   If object created with font data as DKData object,
	///   the data object must not be modified after object created.
    class DKGL_API DKFont
    {
    public:
        struct GlyphData
        {
            DKObject<DKTexture> texture;
            DKPoint	position;
            DKSize	advance;
            DKRect	frame;
        };

		DKFont();
		~DKFont();

		/// create object from file (path)
		static DKObject<DKFont> Create(const DKString& file);
		/// create object from data pointer. (data will be copied inside)
		static DKObject<DKFont> Create(void* data, size_t size);
		/// create object from DKData object. (data must not be modified by outside)
		static DKObject<DKFont> Create(DKData* data);
		/// create object from stream, stream can be copied if necessary.
		static DKObject<DKFont> Create(DKStream* stream);

		const GlyphData* GlyphDataForChar(wchar_t c) const;

		/// text pixel-width from baseline. not includes outline.
		float LineWidth(const DKString& str) const;
		/// pixel-height from baseline. not includes outline.
		float LineHeight() const;

        /// The distance from the baseline to the highest or upper grid coordinate used to place an outline point.
        float Ascender() const;
        /// The distance from the baseline to the lowest grid coordinate used to place an outline point.
        float Descender() const;
		/// text bounding box.
		DKRect Bounds(const DKString& str) const; 
		/// calculate kern advance between characters.
		DKPoint	KernAdvance(wchar_t left, wchar_t right) const;

		int PointSize() const				{ return pointSize; }
		const DKPoint& Resolution() const	{ return resolution; }
		float Outline() const				{ return outline; }
		float Embolden() const				{ return embolden; }
		bool KerningEnabled() const			{ return kerningEnabled; }
		bool ForceBitmap() const			{ return forceBitmap; }

		DKString FamilyName() const;
		DKString StyleName() const;

		/// point, embolden is point-size, outline is pixel-size.
		bool SetStyle(int point, float embolden = 0, float outline = 0, DKPoint dpi = DKPoint(72,72), bool enableKerning = true, bool forceBitmap = false);
		bool IsValid() const;

		float Height() const;	///< font pixel-height (includes outline)
		float Width() const;	///< font pixel-width (includes outline)

		void ClearCache();		///< clear glyph textures.

	private:
		float		outline;			// 0 for no-outline
		float		embolden;			// 0 for regular font
		int			pointSize;			// point size based DPI
		DKPoint		resolution;			// DPI resolution
		bool		kerningEnabled;		// kerning on/off
		bool		forceBitmap;		// force bitmap loads

		struct GlyphTextureAtlas
		{
			DKObject<DKTexture>	texture;

            uint32_t filledVertical;
            uint32_t currentLineWidth;
            uint32_t currentLineMaxHeight;
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

		DKTexture* CacheGlyphTexture(int width, int height, const void* data, DKRect& rect) const;
	};
}
