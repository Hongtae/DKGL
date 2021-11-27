//
//  File: DKFont.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_BBOX_H

#include "DKMath.h"
#include "DKFont.h"

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			// FreeType library.
			class FTLibrary
			{
			public:
				static FT_Library& GetLibrary()
				{
					static FTLibrary	lib;
					return lib.library;
				}
			private:
				FTLibrary()
				{
					FT_Init_FreeType(&library);
				}
				~FTLibrary()
				{
					FT_Done_FreeType(library);
				}
				FT_Library	library;
			};
		}
	}
}

using namespace DKFramework;

DKFont::DKFont()
	: ftFace(nullptr)
	, outline(0)
	, embolden(0)
	, size26d6(10 * 64)
	, dpiX(72)
    , dpiY(72)
	, numGlyphsLoaded(0)
	, forceBitmap(0)
	, kerningEnabled(false)
    , device(nullptr)
{
}

DKFont::~DKFont()
{
	if (ftFace)
		FT_Done_Face(reinterpret_cast<FT_Face>(ftFace));
	if (fontData)
	{
		fontData->UnlockShared();
		fontData = nullptr;
	}
}

void DKFont::SetDevice(DKGraphicsDeviceContext* device)
{
    this->device = device;
    ClearCache();
}

DKObject<DKFont> DKFont::Create(const DKString& file, DKGraphicsDeviceContext* device)
{
	if (file.Length() == 0)
		return nullptr;

	DKStringU8 filename(file);
	if (filename.Bytes() == 0)
		return nullptr;

	FT_Face	face = nullptr;
	FT_Error err = FT_New_Face(Private::FTLibrary::GetLibrary(), (const char*)filename, 0, &face);
	if (err)
	{
		return nullptr;
	}
	if (face->charmap == nullptr)
	{
		if (FT_Set_Charmap(face, face->charmaps[0]))
			return nullptr;
	}

	DKObject<DKFont> font = DKObject<DKFont>::New();
    font->device = device;
	font->ftFace = face;
    if (FT_Set_Char_Size(face, 0, font->size26d6, font->dpiX, font->dpiY))
    {
        DKLogW("Failed to initialize font style, You should call DKFont::SetStyle() manually.");
    }
	return font;
}

DKObject<DKFont> DKFont::Create(const void* data, size_t size, DKGraphicsDeviceContext* device)
{
	if (data && size > 0)
	{
		return Create(DKBuffer::Create(data, size), device);
	}
	return nullptr;
}

DKObject<DKFont> DKFont::Create(DKData* data, DKGraphicsDeviceContext* device)
{
	if (data == nullptr)
		return nullptr;

	const void* ptr = data->LockShared();
	FT_Face	face = nullptr;
	FT_Error err = FT_New_Memory_Face(Private::FTLibrary::GetLibrary(), (const FT_Byte*)ptr, data->Length(), 0, &face);
	if (err == 0)
	{
		if (face->charmap == nullptr)
		{
			if (FT_Set_Charmap(face, face->charmaps[0]))
				return nullptr;
		}

		DKObject<DKFont> font = DKObject<DKFont>::New();
        font->device = device;
		font->ftFace = face;
		font->fontData = data;
        if (FT_Set_Char_Size(face, 0, font->size26d6, font->dpiX, font->dpiY))
        {
            DKLogW("Failed to initialize font style, You should call DKFont::SetStyle() manually.");
        }
        return font;
	}
	data->UnlockShared();
	return nullptr;
}

DKObject<DKFont> DKFont::Create(DKStream* stream, DKGraphicsDeviceContext* device)
{
	if (stream && stream->IsReadable())
	{
		if (stream->IsWritable() == false)	// read only
		{
			DKFile* file = DKObject<DKStream>(stream).SafeCast<DKFile>();
			DKDataStream* ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
			if (file)
			{
				DKObject<DKFont> font = Create(file->Path(), device);
				if (font)
					return font;
			}
			else if (ds)
			{
				DKObject<DKFont> font = Create(const_cast<DKData*>(ds->Data()), device);
				if (font)
					return font;
			}
		}
		return Create(DKBuffer::Create(stream), device);
	}
	return nullptr;
}

const DKFont::GlyphData* DKFont::GlyphDataForChar(wchar_t c) const
{
	if (c == 0 || IsValid() == false)
		return nullptr;

	DKCriticalSection<DKSpinLock> guard(lock);
	const GlyphDataMap::Pair* p = glyphMap.Find(c);
	if (p)
	{
		return &p->value;
	}

	FT_Face face = reinterpret_cast<FT_Face>(ftFace);

	GlyphData	data;
	data.advance = DKSize(0,0);
	data.position = DKPoint(0,0);
	data.texture = nullptr;
	data.frame = DKRect(0,0,0,0);

	unsigned int index = FT_Get_Char_Index(face, c);
	// Loading font.
	FT_Int32	loadFlag = forceBitmap ? FT_LOAD_RENDER : FT_LOAD_DEFAULT;
	if (FT_Load_Glyph(face, index, loadFlag))
	{
		DKLogE("Failed to load glyph for char='%lc'(0x%x)", c, uint32_t(c));
		return nullptr;
	}

	float ascender = this->Ascender();
	FT_Pos boldStrength = embolden * 64.0;
	FT_Pos outlineSize = outline * 64.0;
	data.advance = DKSize(face->glyph->advance.x + boldStrength, face->glyph->advance.y + boldStrength) / 64.0f;

	if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
	{
		face->glyph->outline.flags |= FT_OUTLINE_HIGH_PRECISION;

		if (outline > 0)
		{
			// create outline stroker, drawing outline as bitmap.
			FT_Outline_Embolden(&face->glyph->outline, boldStrength);
			FT_Stroker	stroker;
			FT_Stroker_New(Private::FTLibrary::GetLibrary(), &stroker);
			FT_Stroker_Set(stroker, outlineSize, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
			FT_Stroker_ParseOutline(stroker, &face->glyph->outline, 0);
			FT_Outline	ftOutline;
			FT_UInt points = 0;
			FT_UInt contours = 0;
			FT_Stroker_GetCounts(stroker, &points, &contours);
			FT_Outline_New(Private::FTLibrary::GetLibrary(), points, contours, &ftOutline);
			ftOutline.n_contours = 0;
			ftOutline.n_points = 0;
			FT_Stroker_Export(stroker, &ftOutline);
			FT_Stroker_Done(stroker);

			FT_Bitmap ftBitmap;
			FT_Bitmap_New(&ftBitmap); 

			FT_BBox cbox;
			FT_Outline_Get_CBox(&ftOutline, &cbox); 

			cbox.xMin = cbox.xMin & ~63;
			cbox.yMin = cbox.yMin & ~63;
			cbox.xMax = (cbox.xMax + 63) & ~63;
			cbox.yMax = (cbox.yMax + 63) & ~63;

			FT_UInt	width = (FT_UInt)(cbox.xMax - cbox.xMin) >> 6;
			FT_UInt height = (FT_UInt)(cbox.yMax - cbox.yMin) >> 6;

			FT_Pos	x_shift = (FT_Int)cbox.xMin; 
			FT_Pos	y_shift = (FT_Int)cbox.yMin; 
			FT_Pos	x_left  = (FT_Int)(cbox.xMin >> 6);		// left offset of glyph
			FT_Pos	y_top   = (FT_Int)(cbox.yMax >> 6);		// upper of offset of glyph (height for origin)

			ftBitmap.width		= width;
			ftBitmap.rows		= height;
			ftBitmap.pitch		= width; 
			ftBitmap.num_grays	= 256; 
			ftBitmap.pixel_mode	= FT_PIXEL_MODE_GRAY; 
			size_t bufferSize = ftBitmap.pitch * ftBitmap.rows;
			ftBitmap.buffer		= (unsigned char*)DKMalloc(bufferSize);
			memset(ftBitmap.buffer, 0, bufferSize);

			FT_Outline_Translate(&ftOutline, -x_shift, -y_shift);

			if (FT_Outline_Get_Bitmap(Private::FTLibrary::GetLibrary(), &ftOutline, &ftBitmap) == 0) 
			{
				// x_left: bitmap starting point from origin
				// y_top: height from origin
				data.position = DKPoint(x_left, ascender - y_top); 
				data.texture = CacheGlyphTexture(ftBitmap.width, ftBitmap.rows, ftBitmap.buffer, data.frame);
			}

			DKFree(ftBitmap.buffer);
			ftBitmap.buffer = NULL;
			FT_Bitmap_Done(Private::FTLibrary::GetLibrary(), &ftBitmap);
			FT_Outline_Done(Private::FTLibrary::GetLibrary(), &ftOutline);
		}
		else
		{
			FT_Outline_Embolden(&face->glyph->outline, boldStrength);

			FT_Glyph        glyph = NULL;
			FT_Get_Glyph(face->glyph, &glyph);
			if (FT_Glyph_To_Bitmap(&glyph,  FT_RENDER_MODE_NORMAL, 0, 1) == 0)
			{
				FT_BitmapGlyph  glyph_bitmap = (FT_BitmapGlyph)glyph;
				// bitmap_left: bitmap offset from origin
				// bitmap_top: height from origin
				data.position = DKPoint(glyph_bitmap->left, ascender - glyph_bitmap->top);
				data.texture = CacheGlyphTexture(glyph_bitmap->bitmap.width, glyph_bitmap->bitmap.rows, glyph_bitmap->bitmap.buffer, data.frame);
			}
			FT_Done_Glyph(glyph);
		}
	}
	else
	{
		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) == 0)
		{
			if (outline > 0)
			{
				//outlineSize += 64;
				outlineSize = outlineSize * 2;
				FT_Pos outerSize = boldStrength + outlineSize;
				FT_Pos innerSize = boldStrength - outlineSize;
				// create two bitmaps, generate outline from bigger subtract smaller
				FT_Bitmap	inner, outer;
				FT_Bitmap_New(&inner);
				FT_Bitmap_New(&outer);
				FT_Bitmap_Copy(Private::FTLibrary::GetLibrary(), &face->glyph->bitmap, &inner);
				FT_Bitmap_Copy(Private::FTLibrary::GetLibrary(), &face->glyph->bitmap, &outer);
				FT_Bitmap_Embolden(Private::FTLibrary::GetLibrary(), &inner, innerSize, innerSize);
				FT_Bitmap_Embolden(Private::FTLibrary::GetLibrary(), &outer, outerSize, outerSize);

				unsigned int offsetX = (outer.width - inner.width)/2;
				unsigned int offsetY = (outer.rows - inner.rows)/2;

				for (unsigned int y = 0; y < inner.rows; y++)
				{
					for (unsigned int x = 0; x < inner.width; x++)
					{
						int value1 = outer.buffer[ (y + offsetY) * outer.width + x + offsetX];
						int value2 = inner.buffer[ y * inner.width + x];

						outer.buffer[ (y + offsetY) * outer.width + x + offsetX] = Max<int>(value1 - value2, 0);
					}
				}
				data.position = DKPoint(face->glyph->bitmap_left - outline, ascender - (face->glyph->bitmap_top + outline));
				data.texture = CacheGlyphTexture(outer.width, outer.rows, outer.buffer, data.frame);

				FT_Bitmap_Done(Private::FTLibrary::GetLibrary(), &inner);
				FT_Bitmap_Done(Private::FTLibrary::GetLibrary(), &outer);
			}
			else
			{
				FT_Bitmap_Embolden(Private::FTLibrary::GetLibrary(), &face->glyph->bitmap, boldStrength, boldStrength);
				data.position = DKPoint(face->glyph->bitmap_left, ascender - (face->glyph->bitmap_top + embolden));
				data.texture = CacheGlyphTexture(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, data.frame);
			}
		}
	}

	glyphMap.Update(c, data);
	return &glyphMap.Value(c);
}

DKTexture* DKFont::CacheGlyphTexture(int width, int height, const void* data, DKRect& rect) const
{
    // keep padding between each glyphs.
    if (width <= 0 || height <= 0)
    {
        rect = DKRect(0, 0, 0, 0);
        return nullptr;
    }

    FT_Face face = reinterpret_cast<FT_Face>(ftFace);

    DKObject<DKGraphicsDevice> device = this->device->Device();
    DKObject<DKCommandQueue> queue = this->device->TransferQueue();

    DKObject<DKTexture> tex = nullptr;

    auto updateTexture = [](DKCommandQueue* queue, DKTexture* texture, const DKRect& rect, const void* data)
    {
        uint32_t x = floorf(rect.origin.x + 0.5f);
        uint32_t y = floorf(rect.origin.y + 0.5f);
        uint32_t width = floorf(rect.size.width + 0.5f);
        uint32_t height = floorf(rect.size.height + 0.5f);

        size_t bufferLength = size_t(width) * size_t(height);

        DKGraphicsDevice* device = queue->Device();
        DKObject<DKGpuBuffer> stagingBuffer = device->CreateBuffer(bufferLength, DKGpuBuffer::StorageModeShared, DKCpuCacheModeReadWrite);
        if (stagingBuffer)
        {
            uint8_t* buff = reinterpret_cast<uint8_t*>(stagingBuffer->Contents());
            for (int i = 0; i < height; i++)
            {
                memcpy(&buff[i * width], &((char*)data)[(height - i - 1) * width], width);
            }

            DKObject<DKCommandBuffer> cb = queue->CreateCommandBuffer();
            DKObject<DKCopyCommandEncoder> encoder = cb->CreateCopyCommandEncoder();
            encoder->CopyFromBufferToTexture(stagingBuffer,
                                             { 0, width, height },
                                             texture,
                                             { 0, 0, x, y, 0 },
                                             { width, height, 1 });
            encoder->EndEncoding();
            cb->Commit();
        }
    };

    auto haveEnoughSpace = [](const GlyphTextureAtlas& atlas, int width, int height)
    {
        int32_t texWidth = atlas.texture->Width();
        int32_t texHeight = atlas.texture->Height();

        if (texWidth < width || texHeight < (atlas.filledVertical + height))
            return false;

        if (texWidth < (atlas.currentLineWidth + width))
        {
            if (texHeight < (atlas.filledVertical + atlas.currentLineMaxHeight + height))
                return false; // not enough space.
        }
        return true;
    };

    constexpr int leftMargin = 1;
    constexpr int rightMargin = 1;
    constexpr int topMargin = 1;
    constexpr int bottomMargin = 1;
    constexpr int hPadding = leftMargin + rightMargin;
    constexpr int vPadding = topMargin + bottomMargin;

    bool createNewTexture = true;
    for (int i = 0; i < textures.Count(); i++)
    {
        GlyphTextureAtlas& gta = textures.Value(i);
        if (haveEnoughSpace(gta, width + hPadding, height + vPadding))
        {
            if ((gta.currentLineWidth + width + leftMargin + rightMargin) > gta.texture->Width())
            {
                // move to next line!
                DKASSERT_DEBUG(gta.currentLineMaxHeight > 0);
                gta.filledVertical += gta.currentLineMaxHeight;
                gta.currentLineWidth = 0;
                gta.currentLineMaxHeight = 0;
            }
            rect = DKRect(gta.currentLineWidth + leftMargin, gta.filledVertical + topMargin, width, height);
            updateTexture(queue, gta.texture, rect, data);

            gta.currentLineWidth += width + hPadding;
            if (height + vPadding > gta.currentLineMaxHeight)
                gta.currentLineMaxHeight = height + vPadding;

            tex = gta.texture;
            createNewTexture = false;
            break;
        }
    }
    if (createNewTexture)
    {
        // create new texture.
        uint32_t desiredArea = (Width() + hPadding) * (Height() + vPadding) * (face->num_glyphs - numGlyphsLoaded);
        const uint32_t maxTextureSize = 2048;// 8192;
        const uint32_t minTextureSize = [maxTextureSize](uint32_t minReq) ->uint32_t
        {
			DKASSERT_DEBUG(maxTextureSize > minReq);
            uint32_t size = 32;
            while (size < maxTextureSize && size < minReq) { size = size * 2; }
            return size;
        } (Max(Width() + hPadding, Height() + vPadding));

        uint32_t desiredWidth = minTextureSize;
        uint32_t desiredHeight = minTextureSize;
        while ((desiredWidth * desiredHeight) < desiredArea)
        {
            if (desiredWidth > desiredHeight)
                desiredHeight <<= 1;
            else if (desiredHeight > desiredWidth)
                desiredWidth <<= 1;
            else if (desiredWidth < maxTextureSize)
                desiredWidth <<= 1;
            else if (desiredHeight < maxTextureSize)
                desiredHeight <<= 1;
            else
                break;
        }
        DKLog("Create new texture atlas with resolution: %d x %d", desiredWidth, desiredHeight);
        
        tex = [](int width, int height, DKGraphicsDevice* device)->DKObject<DKTexture>
        {
            // create texture object..
            DKTextureDescriptor desc = {};
            desc.textureType = DKTexture::Type2D;
            desc.pixelFormat = DKPixelFormat::RGBA8Unorm;
            desc.width = width;
            desc.height = height;
            desc.depth = 1;
            desc.mipmapLevels = 1;
            desc.sampleCount = 1;
            desc.arrayLength = 1;
            desc.usage = DKTexture::UsageCopyDestination | DKTexture::UsageSampled;
			return device->CreateTexture(desc);
        }(desiredWidth, desiredHeight, device);

        rect = DKRect(leftMargin, topMargin, width, height);
        updateTexture(queue, tex, rect, data);

        GlyphTextureAtlas gta = {
            tex,
            0,
            static_cast<uint32_t>(width + hPadding),
            static_cast<uint32_t>(height + vPadding)
        };
        textures.Add(gta);
    }
    numGlyphsLoaded++;

    return tex;
}

float DKFont::Ascender() const
{
    FT_Face face = reinterpret_cast<FT_Face>(ftFace);
    if (face->size == 0)
        return 0;

    float baseline = 0;
    if (FT_IS_SCALABLE(face))
    {
        baseline = (face->bbox.yMax) * (float)face->size->metrics.y_ppem / (float)face->units_per_EM;
    }
    else
    {
        baseline = static_cast<float>(face->size->metrics.height) / 64.0f;
    }
    return floorf(baseline + embolden);
}

float DKFont::Descender() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face->size == 0)
		return 0;

	float descender = 0;
	if (FT_IS_SCALABLE(face))
	{
		descender = ( 0 - face->bbox.yMin ) * (float)face->size->metrics.y_ppem / (float)face->units_per_EM;
	}
	else
	{
		descender = 0;
	}
	return floorf(descender + embolden);
}

float DKFont::Height() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face->size == 0)
		return 0;

	float height = 0;

	if (FT_IS_SCALABLE(face))
	{
		height = (face->bbox.yMax - face->bbox.yMin) * (static_cast<float>(face->size->metrics.y_ppem) / static_cast<float>(face->units_per_EM));
	}
	else
	{
		height = static_cast<float>(face->size->metrics.height) / 64.0f;
	}
	//return height + embolden * 2.0f + (outline > 0.0f ? (outline+1.0f) * 2.0f : 0.0f);
	return ceilf(height + (embolden + outline) * 2.0f);
}

float DKFont::Width() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face->size == 0)
		return 0;

	float width = 0;
	if (FT_IS_SCALABLE(face))
	{
		width = (face->bbox.xMax - face->bbox.xMin) * (static_cast<float>(face->size->metrics.x_ppem) / static_cast<float>(face->units_per_EM));
	}
	else
	{
		width = static_cast<float>(face->size->metrics.max_advance) / 64.0f;
	}

	//return width + embolden * 2.0f + (outline > 0.0f ? (outline+1.0f) * 2.0f : 0.0f);
	return ceilf(width + (embolden + outline) * 2.0f);
}

float DKFont::LineHeight() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face->size == 0)
		return 0;

	float height = 0;

	if (FT_IS_SCALABLE(face))
	{
		height = (face->bbox.yMax - face->bbox.yMin) * (static_cast<float>(face->size->metrics.y_ppem) / static_cast<float>(face->units_per_EM));
	}
	else
	{
		height = static_cast<float>(face->size->metrics.height) / 64.0f;
	}
	//return height + embolden * 2.0f;
	return ceilf(height + embolden * 2.0f);
}

float DKFont::LineWidth(const DKString& str) const
{
	size_t len = str.Length();
	float lineLength = 0;

	for (size_t i = 0; i < len; ++i)
	{
		const GlyphData* glyph = GlyphDataForChar(str[i]);
		if (glyph)
		{
			lineLength += glyph->advance.width + KernAdvance(str[i], str[i + 1]).x;
		}
	}
	//return lineLength;
	return ceilf(lineLength);
}

DKRect DKFont::Bounds(const DKString& str) const
{
	DKPoint bboxMin(0, 0);
	DKPoint bboxMax(0, 0);
	float offset = 0;
	const size_t len = str.Length();

	for (size_t i = 0; i < len; ++i)
	{
		const GlyphData* glyph = GlyphDataForChar(str[i]);
		if (glyph == NULL)
			continue;

        if (offset > 0)
        {
            const DKPoint posMin(offset + glyph->position.x, glyph->position.y);
            const DKPoint posMax(posMin + glyph->frame.size.Vector());

            if (bboxMin.x > posMin.x)	bboxMin.x = posMin.x;
            if (bboxMin.y > posMin.y)	bboxMin.y = posMin.y;
            if (bboxMax.x < posMax.x)	bboxMax.x = posMax.x;
            if (bboxMax.y < posMax.y)	bboxMax.y = posMax.y;
        }
		else
		{
            bboxMin = glyph->position;
            bboxMax = glyph->position + glyph->frame.size.Vector();
		}

		offset += glyph->advance.width + KernAdvance(str[i], str[i+1]).x;
	}

	//return DKRect(bboxMin, DKSize(bboxMax.x - bboxMin.x, bboxMax.y - bboxMin.y));
	const DKSize size = DKSize(ceilf(bboxMax.x - bboxMin.x), ceilf(bboxMax.y - bboxMin.y));
	const DKPoint origin = DKPoint(bboxMin.x, bboxMin.y);
	return DKRect(origin, size);
}

DKPoint	DKFont::KernAdvance(wchar_t left, wchar_t right) const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);

	DKPoint	ret(0,0);

	if (this->kerningEnabled && FT_HAS_KERNING(face))
	{
		DKCriticalSection<DKSpinLock> guard(lock);

		unsigned int index1 = 0;
		const CharIndexMap::Pair* pLeft = this->charIndexMap.Find(left);
		if (pLeft)
		{
			index1 = pLeft->value;
		}
		else
		{
			index1 = FT_Get_Char_Index(face, left);
			this->charIndexMap.Update(left, index1);
		}
		unsigned int index2 = 0;
		const CharIndexMap::Pair* pRight = this->charIndexMap.Find(right);
		if (pRight)
		{
			index2 = pRight->value;
		}
		else
		{
			index2 = FT_Get_Char_Index(face, right);
			this->charIndexMap.Update(right, index2);
		}

		if (index1 && index2)
		{
			FT_Vector kernAdvance = {0, 0};
			if (FT_Get_Kerning(face, index1, index2, FT_KERNING_DEFAULT, &kernAdvance) == 0)
			{
				ret.x = static_cast<float>( kernAdvance.x ) / 64.0f;
				ret.y = static_cast<float>( kernAdvance.y ) / 64.0f;
			}
		}
	}
	return ret;
}

bool DKFont::SetStyle(float point, uint32_t resX, uint32_t resY, float embolden, float outline, bool enableKerning, bool forceBitmap)
{
	if (ftFace == NULL)
		return false;
	if (embolden < 0)
		embolden = 0;
	if (outline < 0)
		outline = 0;
    if (resX < 1)
        resX = 1;
    if (resY < 1)
        resY = 1;

	//for (int i = 0; i < reinterpret_cast<FT_Face>(ftFace)->num_fixed_sizes; i++)
	//{
	//	DKLog("available size: %d\n",reinterpret_cast<FT_Face>(ftFace)->available_sizes[i].size);
	//}

    // clamp pointSize (26.6 signed-fixed) from 1/64 to 2^25-(1/64)
    double dp = Clamp<double>(double(point) * 64.0, 1.0, double(0x7fffffffu));
    FT_F26Dot6 charSize = floor(dp);

	if (size26d6 != charSize ||
		this->embolden != embolden ||
		this->outline != outline ||
		dpiX != resX ||
		dpiY != resY ||
		this->forceBitmap != forceBitmap)
	{
        DKCriticalSection<DKSpinLock> guard(lock);
        if (size26d6 != charSize || dpiX != resX || dpiY != resY)
        {
            if (FT_Set_Char_Size(reinterpret_cast<FT_Face>(ftFace), 0, charSize, resX, resY) == 0)
            {
                DKLogE("FT_Set_Char_Size failed! (size:0x%x, dpi:%ux%u)",
                    charSize, resX, resY);
                return false;
            }
        }

        size26d6 = charSize;
        dpiX = resX;
        dpiY = resY;

        glyphMap.Clear();
        charIndexMap.Clear();
        textures.Clear();
        numGlyphsLoaded = 0;

        this->outline = outline;
        this->embolden = embolden;
        this->forceBitmap = forceBitmap;
        this->kerningEnabled = enableKerning;

        return true;
	}
	this->kerningEnabled = enableKerning;
	return true;
}

void DKFont::ClearCache()
{
	DKCriticalSection<DKSpinLock> guard(lock);
	glyphMap.Clear();
	charIndexMap.Clear();
	textures.Clear();
	numGlyphsLoaded = 0;
}

bool DKFont::IsValid() const
{
	if (ftFace && size26d6 > 0)
		return true;
	return false;
}

DKString DKFont::FamilyName() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face)
		return DKString(face->family_name);
	return L"";
}

DKString DKFont::StyleName() const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face)
		return DKString(face->style_name);
	return L"";
}

