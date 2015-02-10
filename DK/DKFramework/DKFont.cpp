//
//  File: DKFont.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#define DKLIB_EXTDEPS_FREETYPE
#include "../lib/ExtDeps.h"
#include "../lib/OpenGL.h"
#include "DKMath.h"
#include "DKFont.h"
#include "DKTexture2D.h"

using namespace DKFoundation;
using namespace DKFramework;

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
				static FT_Library& GetLibrary(void)
				{
					static FTLibrary	lib;
					return lib.library;
				}
			private:
				FTLibrary(void)
				{
					FT_Init_FreeType(&library);
				}
				~FTLibrary(void)
				{
					FT_Done_FreeType(library);
				}
				FT_Library	library;
			};
		}
	}
}

DKFont::DKFont(void)
	: ftFace(NULL)
	, outline(0)
	, embolden(0)
	, pointSize(0)
	, resolution(72, 72)
	, numGlyphsLoaded(0)
	, forceBitmap(0)
	, kerningEnabled(false)
{
}

DKFont::~DKFont(void)
{
	if (ftFace)
		FT_Done_Face(reinterpret_cast<FT_Face>(ftFace));
	if (fontData)
	{
		fontData->UnlockShared();
		fontData = NULL;
	}
}

DKObject<DKFont> DKFont::Create(const DKString& file)
{
	if (file.Length() == 0)
		return NULL;

	DKStringU8 filename(file);
	if (filename.Bytes() == 0)
		return NULL;

	FT_Face	face = NULL;
	FT_Error err = FT_New_Face(Private::FTLibrary::GetLibrary(), (const char*)filename, 0, &face);
	if (err)
	{
		return NULL;
	}
	if (face->charmap == NULL)
	{
		if (FT_Set_Charmap(face, face->charmaps[0]))
			return NULL;
	}

	DKObject<DKFont> font = DKObject<DKFont>::New();
	font->ftFace = face;
	return font;
}

DKObject<DKFont> DKFont::Create(void* data, size_t size)
{
	if (data && size > 0)
	{
		return Create(DKBuffer::Create(data, size));
	}
	return NULL;
}

DKObject<DKFont> DKFont::Create(DKData* data)
{
	if (data == NULL)
		return NULL;

	const void* ptr = data->LockShared();
	FT_Face	face = NULL;
	FT_Error err = FT_New_Memory_Face(Private::FTLibrary::GetLibrary(), (const FT_Byte*)ptr, data->Length(), 0, &face);
	if (err == 0)
	{
		if (face->charmap == NULL)
		{
			if (FT_Set_Charmap(face, face->charmaps[0]))
				return NULL;
		}

		DKObject<DKFont> font = DKObject<DKFont>::New();
		font->ftFace = face;
		font->fontData = data;
		return font;
	}
	data->UnlockShared();
	return NULL;
}

DKObject<DKFont> DKFont::Create(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		if (stream->IsWritable() == false)	// read only
		{
			DKFile* file = DKObject<DKStream>(stream).SafeCast<DKFile>();
			DKDataStream* ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
			if (file)
			{
				DKObject<DKFont> font = Create(file->Path());
				if (font)
					return font;
			}
			else if (ds)
			{
				DKObject<DKFont> font = Create(const_cast<DKData*>(ds->DataSource()));
				if (font)
					return font;
			}
		}
		return Create(DKBuffer::Create(stream));
	}
	return NULL;
}

const DKFont::GlyphData* DKFont::GetGlyphData(wchar_t c) const
{
	if (c == 0 || IsValid() == false)
		return NULL;

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
	data.texture = NULL;
	data.rect = DKRect(0,0,0,0);

	unsigned int index = FT_Get_Char_Index(face, c);
	// Loading font.
	FT_Int32	loadFlag = forceBitmap ? FT_LOAD_RENDER : FT_LOAD_DEFAULT;
	if (FT_Load_Glyph(face, index, loadFlag))
	{
		DKLog("Failed to load glyph for char='%lc'\n", c);
		return NULL;
	}

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
			ftBitmap.buffer		= (unsigned char*)DKMemoryHeapAlloc(bufferSize);
			memset(ftBitmap.buffer, 0, bufferSize);

			FT_Outline_Translate(&ftOutline, -x_shift, -y_shift);

			if (FT_Outline_Get_Bitmap(Private::FTLibrary::GetLibrary(), &ftOutline, &ftBitmap) == 0) 
			{
				// x_left: bitmap starting point from origin
				// y_top: height from origin
				data.position = DKPoint(x_left, y_top - ftBitmap.rows); 
				data.texture = GetGlyphTexture(ftBitmap.width, ftBitmap.rows, ftBitmap.buffer, data.rect); 
			}

			DKMemoryHeapFree(ftBitmap.buffer);
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
				data.position = DKPoint(glyph_bitmap->left, glyph_bitmap->top - glyph_bitmap->bitmap.rows);
				data.texture = GetGlyphTexture(glyph_bitmap->bitmap.width, glyph_bitmap->bitmap.rows, glyph_bitmap->bitmap.buffer, data.rect);
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

				for (int y = 0; y < inner.rows; y++)
				{
					for (int x = 0; x < inner.width; x++)
					{
						int value1 = outer.buffer[ (y + offsetY) * outer.width + x + offsetX];
						int value2 = inner.buffer[ y * inner.width + x];

						outer.buffer[ (y + offsetY) * outer.width + x + offsetX] = Max<int>(value1 - value2, 0);
					}
				}
				data.position = DKPoint(face->glyph->bitmap_left - outline, face->glyph->bitmap_top - face->glyph->bitmap.rows - outline);
				data.texture = GetGlyphTexture(outer.width, outer.rows, outer.buffer, data.rect); 

				FT_Bitmap_Done(Private::FTLibrary::GetLibrary(), &inner);
				FT_Bitmap_Done(Private::FTLibrary::GetLibrary(), &outer);
			}
			else
			{
				FT_Bitmap_Embolden(Private::FTLibrary::GetLibrary(), &face->glyph->bitmap, boldStrength, boldStrength);
				data.position = DKPoint(face->glyph->bitmap_left, face->glyph->bitmap_top - face->glyph->bitmap.rows + embolden); 
				data.texture = GetGlyphTexture(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, data.rect); 
			}
		}
	}

	glyphMap.Update(c, data);
	return &glyphMap.Value(c);
}

DKTexture2D* DKFont::GetGlyphTexture(int width, int height, void* data, DKRect& rect) const
{
	// keep padding between each glyphs.
	if (width <= 0 || height <= 0)
	{
		rect = DKRect(0,0,0,0);
		return NULL;
	}

	FT_Face face = reinterpret_cast<FT_Face>(ftFace);

	DKObject<DKTexture2D> tex = NULL;

	char* buff = (char*)DKMemoryHeapAlloc(width * height);
	for (int i = 0; i < height; i++)
	{
		memcpy(&buff[i * width] , &((char*)data)[(height - i -1) * width], width);
	}

	const int padding = 1;		// padding for right, bottom of glyph

	bool createNewTexture = true;
	for (int i = 0; i < textures.Count(); i++)
	{
		SharedTextures& st = textures.Value(i);
		if (st.freeSpaceWidth >= (width + padding))
		{
			rect = DKRect(st.texture->Width() - st.freeSpaceWidth, 0, width, height);
			st.texture->SetPixelData(rect, buff);
			DKASSERT_DEBUG((width + padding) <= st.freeSpaceWidth);
			st.freeSpaceWidth -= width + padding; // add padding next to glyph
			tex = st.texture;
			createNewTexture = false;
			break;
		}
	}
	if (createNewTexture)
	{
		// create new texture.
		int reqWidth = (Width() + padding) * (face->num_glyphs - numGlyphsLoaded); // padding for each glyphs.
		static int maxTextureSize = 0;
		if (maxTextureSize == 0)
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

		int newTexWidth = Min(reqWidth, maxTextureSize);

		DKASSERT_DEBUG(newTexWidth >= width);

		int newTexHeight = Height() + padding;

		void* initialData = DKMemoryHeapAlloc(newTexWidth * newTexHeight);
		memset(initialData, 0, newTexWidth * newTexHeight);
		tex = DKTexture2D::Create(newTexWidth, newTexHeight, DKTexture::FormatAlpha, DKTexture::TypeUnsignedByte, initialData);
		DKMemoryHeapFree(initialData);

		rect = DKRect(0,0,width,height);
		tex->SetPixelData(rect, buff);
		DKASSERT_DEBUG((newTexWidth - width - padding) > 0);
		SharedTextures st = {tex, static_cast<unsigned int>(newTexWidth - width - padding)};
		textures.Add(st);
	}
	DKMemoryHeapFree(buff);
	numGlyphsLoaded++;

	return tex;
}

float DKFont::Baseline(void) const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face->size == 0)
		return 0;

	float baseline = 0;
	if (FT_IS_SCALABLE(face))
	{
		baseline = ( 0 - face->bbox.yMin ) * (float)face->size->metrics.y_ppem / (float)face->units_per_EM;
	}
	else
	{
		baseline = 0;
	}
	//return baseline + embolden + (outline > 0.0f ? (outline+1.0f) : 0.0f);
	//return ceilf(baseline + embolden + (outline > 0.0f ? (outline+1.0f) : 0.0f));
	return ceilf(baseline + embolden);
}

float DKFont::Height(void) const
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
	return ceilf(height + embolden * 2.0f + (outline > 0.0f ? (outline+1.0f) * 2.0f : 0.0f));
}

float DKFont::Width(void) const
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
	return ceilf(width + embolden * 2.0f + (outline > 0.0f ? (outline+1.0f) * 2.0f : 0.0f));
}

float DKFont::LineHeight(void) const
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
		const GlyphData* glyph = GetGlyphData(str[i]);
		if (glyph)
		{
			lineLength += KernAdvance(str[i], str[i+1]).x + glyph->advance.width;
		}
	}
	//return lineLength;
	return ceilf(lineLength);
}

DKRect DKFont::Bounds(const DKFoundation::DKString& str) const
{
	DKPoint bboxMin(0, 0);
	DKPoint bboxMax(0, 0);
	float offset = 0;
	size_t len = str.Length();

	for (size_t i = 0; i < len; ++i)
	{
		const GlyphData* glyph = GetGlyphData(str[i]);
		if (glyph == NULL)
			continue;

		DKPoint posMin(offset + glyph->position.x, glyph->position.y);
		DKPoint posMax(offset + glyph->position.x + glyph->rect.size.width, glyph->position.y + glyph->rect.size.height);

		if (bboxMin.x > posMin.x)	bboxMin.x = posMin.x;
		if (bboxMin.y > posMin.y)	bboxMin.y = posMin.y;
		if (bboxMax.x < posMax.x)	bboxMax.x = posMax.x;
		if (bboxMax.y < posMax.y)	bboxMax.y = posMax.y;

		offset += glyph->advance.width + KernAdvance(str[i], str[i+1]).x;
	}

	//return DKRect(bboxMin, DKSize(bboxMax.x - bboxMin.x, bboxMax.y - bboxMin.y));
	DKSize size = DKSize(ceilf(bboxMax.x - bboxMin.x), ceilf(bboxMax.y - bboxMin.y));
	DKPoint origin = DKPoint(floor(bboxMin.x), floor(bboxMin.y));
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

bool DKFont::SetStyle(int point, float embolden, float outline, DKPoint dpi, bool enableKerning, bool forceBitmap)
{
	if (dpi.x <= 0 || dpi.y <= 0)
		return false;
	if (ftFace == NULL)
		return false;
	if (point <= 0)
		return false;
	if (embolden < 0)
		embolden = 0;
	if (outline < 0)
		outline = 0;

	//for (int i = 0; i < reinterpret_cast<FT_Face>(ftFace)->num_fixed_sizes; i++)
	//{
	//	DKLog("available size: %d\n",reinterpret_cast<FT_Face>(ftFace)->available_sizes[i].size);
	//}

	FT_UInt resX2 = floor(dpi.x + 0.5f);
	FT_UInt resY2 = floor(dpi.y + 0.5f);

	if (resX2 <= 0 || resY2 <= 0)
		return false;

	FT_UInt resX1 = floor(this->resolution.x + 0.5f);
	FT_UInt resY1 = floor(this->resolution.y + 0.5f);


	if (pointSize != point ||
		this->embolden != embolden ||
		this->outline != outline ||
		resX1 != resX2 ||
		resY1 != resY2 ||
		this->forceBitmap != forceBitmap)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		if (FT_Set_Char_Size(reinterpret_cast<FT_Face>(ftFace), 0, point * 64, resX2, resY2) == 0)
		{
			resolution = DKPoint(resX2, resY2);
			pointSize = point;

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
		return false;
	}
	this->resolution = DKPoint(resX2, resY2);
	this->kerningEnabled = enableKerning;
	return true;
}

bool DKFont::IsValid(void) const
{
	if (ftFace && pointSize > 0)
		return true;
	return false;
}

DKString DKFont::FamilyName(void) const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face)
		return DKString(face->family_name);
	return L"";
}

DKString DKFont::StyleName(void) const
{
	FT_Face face = reinterpret_cast<FT_Face>(ftFace);
	if (face)
		return DKString(face->style_name);
	return L"";
}

