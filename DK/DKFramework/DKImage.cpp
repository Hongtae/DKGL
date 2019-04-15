//
//  File: DKImage.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#include "../Libs/libpng/png.h"
#include "../Libs/jpeg/jpeglib.h"

#include "DKImage.h"
#include "DKResourceLoader.h"

#define JPEG_BUFFER_SIZE	4096
#define BMP_DEFAULT_PPM		96

namespace DKFramework::Private
{
#pragma pack(push, 1)
    enum BMPCompression : uint32_t
    {
        BMPCompressionRGB = 0L,
        BMPCompressionRLE8 = 1L,
        BMPCompressionRLE4 = 2L,
        BMPCompressionBITFIELDS = 3L,
    };
    struct BMPFileHeader // little-endian
    {
        uint8_t    b; // = 'B'
        uint8_t    m; // = 'M'
        uint32_t   size;
        uint16_t   reserved1;
        uint16_t   reserved2;
        uint32_t   offBits;
    };
    struct BMPCoreHeader
    {
        uint32_t   size;
        uint16_t   width;
        uint16_t   height;
        uint16_t   planes;
        uint16_t   bitCount;
    };
    static_assert(sizeof(BMPFileHeader) == 14, "Wrong BMP header!");
    static_assert(sizeof(BMPCoreHeader) == 12, "Wrong BMP header!");

    struct BMPInfoHeader
    {
        uint32_t   size;
        int32_t    width;
        int32_t    height;
        uint16_t   planes;
        uint16_t   bitCount;
        uint32_t   compression;
        uint32_t   sizeImage;
        int32_t    xPelsPerMeter;
        int32_t    yPelsPerMeter;
        uint32_t   clrUsed;
        uint32_t   clrImportant;
    };
#pragma pack(pop)

    struct JpegErrorMgr
    {
        struct jpeg_error_mgr pub;
        jmp_buf setjmpBuffer;
        char buffer[JMSG_LENGTH_MAX];
    };

    enum ImageFormat
    {
        FormatPNG,
        FormatJPEG,
        FormatBMP
    };

    inline bool IsFormatValid(DKImage::PixelFormat format)
    {
        switch (format)
        {
        case DKImage::R8:
        case DKImage::RG8:
        case DKImage::RGB8:
        case DKImage::RGBA8:
        case DKImage::R16:
        case DKImage::RG16:
        case DKImage::RGB16:
        case DKImage::RGBA16:
        case DKImage::R32:
        case DKImage::RG32:
        case DKImage::RGB32:
        case DKImage::RGBA32:
        case DKImage::R32F:
        case DKImage::RG32F:
        case DKImage::RGB32F:
        case DKImage::RGBA32F:
            return true;
        }
        return false;
    }

    inline bool IdentifyImageFormatFromString(const DKString& str, ImageFormat& f)
    {
        DKString str2 = str.LowercaseString();
        struct {
            const char* suffix;
            ImageFormat format;
        } suffixFormats[] =
        {
            { ".png", FormatPNG },
            { ".jpg", FormatJPEG },
            { ".jpe", FormatJPEG },
            { ".jpeg", FormatJPEG },
            { ".bmp", FormatBMP },
        };
        struct {
            const char* ext;
            ImageFormat format;
        } extFormats[] =
        {
            { "png", FormatPNG },
            { "jpg", FormatJPEG },
            { "jpe", FormatJPEG },
            { "jpeg", FormatJPEG },
            { "bmp", FormatBMP },
            { "image/png", FormatPNG },
            { "image/jpeg", FormatJPEG },
            { "image/bmp", FormatBMP },
        };
        for (auto& fmt : suffixFormats)
        {
            if (str2.HasSuffix(fmt.suffix))
            {
                f = fmt.format;
                return true;
            }
        }
        for (auto& fmt : extFormats)
        {
            if (str2.Compare(fmt.ext) == 0)
            {
                f = fmt.format;
                return true;
            }
        }
        return false;
    }

    inline bool IdentifyImageFormatFromHeader(const uint8_t* p, size_t s, ImageFormat& f)
    {
        if (p && s > 0)
        {
            if (s >= sizeof(BMPFileHeader))
            {
                if (p[0] == 'B' && p[1] == 'M')
                {
                    f = FormatBMP;
                    return true;
                }
            }
            if (s >= 8)
            {
                const png_byte png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
                bool png = true;
                for (int i = 0; i < 8; ++i)
                {
                    if (png_signature[i] != p[i])
                        png = false;
                    break;
                }
                if (png)
                {
                    f = FormatPNG;
                    return true;
                }
            }
            if (s > 3)
            {
                if (p[0] == 0xff && p[1] == 0xd8 && p[2] == 0xff)
                {
                    f = FormatJPEG;
                    return true;
                }
            }
        }
        return false;
    };

    inline size_t BytesPerPixel(DKImage::PixelFormat format)
    {
        switch (format)
        {
        case DKImage::R8:		return 1;
        case DKImage::RG8:		return 2;
        case DKImage::RGB8:		return 3;
        case DKImage::RGBA8:	return 4;
        case DKImage::R16:		return 2;
        case DKImage::RG16:		return 4;
        case DKImage::RGB16:	return 6;
        case DKImage::RGBA16:	return 8;
        case DKImage::R32:		return 4;
        case DKImage::RG32:		return 8;
        case DKImage::RGB32:	return 12;
        case DKImage::RGBA32:	return 16;
        case DKImage::R32F:		return 4;
        case DKImage::RG32F:	return 8;
        case DKImage::RGB32F:	return 12;
        case DKImage::RGBA32F:	return 16;
        }
        return 0;
    }

    // register image extensions to resource-loader.
    int RegisterImageFileExts()
    {
        const wchar_t* exts[] = {
            L"bmp",
            L"jpg", L"jpeg", L"jpe",
            L"png",
            L"tga",
            NULL };

        auto loaderProc = [](DKStream * stream, DKAllocator&)->DKObject<DKResource>
        {
            return DKImage::Create(stream).SafeCast<DKResource>();
        };
        DKObject<DKResourceLoader::ResourceLoader> loader = DKFunction(loaderProc);

        int num = 0;
        while (exts[num])
        {
            DKResourceLoader::SetResourceFileExtension(exts[num], loader);
            num++;
        }
        return num;
    }
    // make sure to allocator exists before register ext-types.
    static DKAllocator::Maintainer init;
    int numRegisteredImageExts = RegisterImageFileExts();
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKImage::DKImage()
	: data(NULL)
	, width(0)
	, height(0)
	, format(R8)
{
}

DKImage::~DKImage()
{
	if (data)
		DKFree(data);
}

size_t DKImage::BytesPerPixel() const
{
	size_t bpp = Private::BytesPerPixel(format);
	DKASSERT_DESC_DEBUG(bpp, "Invalid format");
	return bpp;
}

bool DKImage::IsValid() const
{
	if (width > 0 && height > 0 && data)
	{
		return IsFormatValid(format);
	}
	return false;
}

const void* DKImage::Contents() const
{
    if (IsValid())
        return data;
    return nullptr;
}

DKObject<DKImage> DKImage::Create(uint32_t w, uint32_t h, PixelFormat fmt, const void* p)
{
	size_t bpp = Private::BytesPerPixel(fmt);
	if (bpp && w && h)
	{
		size_t imageSize = w * h * bpp;

		void* imageData = DKMalloc(imageSize);
		if (imageData)
		{
			DKObject<DKImage> image = DKOBJECT_NEW DKImage();
			image->data = imageData;
			image->width = w;
			image->height = h;
			image->format = fmt;

			if (p)
			{
				memcpy(image->data, p, imageSize);
			}
			else
			{
				memset(image->data, 0, imageSize);
			}
			return image;
		}
		else
		{
			DKLogE("Out of memory!");
		}
	}
	else
	{
		DKLogE("DKImage error: Invalid format!");
	}
	return NULL;
}

DKObject<DKImage> DKImage::Create(const DKString &path)
{
	if (path.Length() == 0)
		return NULL;

	DKObject<DKFileMap> file = DKFileMap::Open(path, 0, false);
	if (file)
		return Create(file.SafeCast<DKData>());
	return NULL;
}

DKObject<DKImage> DKImage::Create(DKStream* stream)
{
	DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
	if (ds)
		return Create(ds->Data());
	return Create(DKBuffer::Create(stream));
}

DKObject<DKImage> DKImage::Create(DKData* data)
{
	DKObject<DKData> d2 = data;
	DKDataReader reader(data);
	return Create(reader.Bytes(), reader.Length());
}

DKObject<DKImage> DKImage::Create(const void *p, size_t s)
{
	if (p == NULL || s == 0)
		return NULL;

	ImageFormat fmt;
	if (!IdentifyImageFormatFromHeader(reinterpret_cast<const uint8_t*>(p), s, fmt))
	{
		DKLogE("[DKImage::Create] Error: Unable to identify image format!");
		return NULL;
	}

	switch (fmt)
	{
	case FormatPNG:
		if (1)
		{
			png_image image = {};
			image.version = PNG_IMAGE_VERSION;
			if (png_image_begin_read_from_memory(&image, p, s))
			{
				PixelFormat pixelFormat;
				switch (image.format)
				{
				case PNG_FORMAT_GRAY:
					pixelFormat = R8;
					break;
				case PNG_FORMAT_AG:
					image.format = PNG_FORMAT_GA;
				case PNG_FORMAT_GA:
					pixelFormat = RG8;
					break;
				case PNG_FORMAT_BGR:
					image.format = PNG_FORMAT_RGB;
				case PNG_FORMAT_RGB:
					pixelFormat = RGB8;
					break;
				case PNG_FORMAT_ARGB:
				case PNG_FORMAT_BGRA:
				case PNG_FORMAT_ABGR:
					image.format = PNG_FORMAT_RGBA;
				case PNG_FORMAT_RGBA:
					pixelFormat = RGBA8;
					break;
				case PNG_FORMAT_LINEAR_Y:
					pixelFormat = R16;
					break;
				case PNG_FORMAT_LINEAR_Y_ALPHA:
					pixelFormat = RG16;
					break;
				case PNG_FORMAT_LINEAR_RGB:
					pixelFormat = RGB16;
					break;
				case PNG_FORMAT_LINEAR_RGB_ALPHA:
					pixelFormat = RGBA16;
					break;
				default:
					image.format = PNG_FORMAT_RGBA;
					//png_image_free(&image);
					//return false;
				}

				DKObject<DKImage> output = DKImage::Create(image.width, image.height, pixelFormat, nullptr);
				if (output)
				{
					size_t imageSizeInBytes = PNG_IMAGE_SIZE(image);
					if (imageSizeInBytes == output->BytesPerPixel() * output->width * output->height)
					{
						if (png_image_finish_read(&image, nullptr, output->data, 0, nullptr))
							return output;
					}
					output = NULL;
					DKLogE("[DKImage::Create] Error: PNG buffer error!");
				}
				else
				{
					DKLogE("[DKImage::Create] Error: Out of memory!");
				}
				// failed! (dealloc buffer and return)
				png_image_free(&image);
			}
		}
		break;
	case FormatJPEG:
		if (1)
		{
			jpeg_decompress_struct cinfo = {};
			JpegErrorMgr err = {};
			struct JpegSource
			{
				struct jpeg_source_mgr pub;
				const uint8_t* data;
				size_t length;
				uint8_t eofMarker[2];
			};
			JpegSource source;
			source.data = reinterpret_cast<const uint8_t*>(p);
			source.length = s;
			source.pub.bytes_in_buffer = 0;
			source.pub.next_input_byte = NULL;
			source.pub.init_source = [](j_decompress_ptr cinfo) {};
			source.pub.fill_input_buffer = [](j_decompress_ptr cinfo)->boolean
			{
				JpegSource* src = reinterpret_cast<JpegSource*>(cinfo->src);
				if (src->length > 0)
				{
					src->pub.next_input_byte = (const JOCTET*)src->data;
					size_t s = Min(src->length, size_t(JPEG_BUFFER_SIZE));
					src->pub.bytes_in_buffer = s;
					src->length -= s;
					src->data += s;
				}
				else
				{
					src->pub.next_input_byte = (const JOCTET*)src->eofMarker;
					src->eofMarker[0] = 0xff;
					src->eofMarker[1] = JPEG_EOI;
					src->pub.bytes_in_buffer = 2;
				}
				return TRUE;
			};
			source.pub.skip_input_data = [](j_decompress_ptr cinfo, long numBytes)
			{
				if (numBytes > 0)
				{
					JpegSource* src = reinterpret_cast<JpegSource*>(cinfo->src);
					while (numBytes > (long)src->pub.bytes_in_buffer)
					{
						numBytes -= (long)src->pub.bytes_in_buffer;
						src->pub.fill_input_buffer(cinfo);
					}
					src->pub.next_input_byte += (size_t)numBytes;
					src->pub.bytes_in_buffer -= (size_t)numBytes;

				}
			};
			source.pub.term_source = [](j_decompress_ptr cinfo) {};

			cinfo.err = jpeg_std_error(&err.pub);
			err.pub.error_exit = [](j_common_ptr cinfo)
			{
				JpegErrorMgr* err = (JpegErrorMgr*)cinfo->err;
				err->pub.format_message(cinfo, err->buffer);
				longjmp(err->setjmpBuffer, 1);
			};

			if (setjmp(err.setjmpBuffer))
			{
				DKLogE("[DKImage::Create] JPEG Error: %s", err.buffer);
				jpeg_destroy_decompress(&cinfo);
				return NULL;
			}
			jpeg_create_decompress(&cinfo);
			cinfo.src = (jpeg_source_mgr*)&source;
			jpeg_read_header(&cinfo, TRUE);

			int32_t bytesPerPixel;
			if (cinfo.out_color_space == JCS_CMYK || cinfo.out_color_space == JCS_YCCK)
			{
				cinfo.out_color_space = JCS_CMYK;
				bytesPerPixel = 4;
			}
			else
			{
				cinfo.out_color_space = JCS_RGB;
				bytesPerPixel = 3;
			}
			jpeg_start_decompress(&cinfo);

			DKObject<DKImage> output = DKImage::Create(cinfo.image_width, cinfo.image_height, RGB8, 0);
			if (output)
			{
				uint32_t rowStride = output->width * 3;
				uint8_t* data = (uint8_t*)output->data;

				if (cinfo.out_color_space == JCS_RGB)
				{
					JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
						((j_common_ptr)&cinfo, JPOOL_IMAGE, rowStride, 1);
					while (cinfo.output_scanline < cinfo.output_height)
					{
						jpeg_read_scanlines(&cinfo, buffer, 1);
						memcpy(data, buffer[0], rowStride);
						data += rowStride;
					}
				}
				else
				{
					JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
						((j_common_ptr)&cinfo, JPOOL_IMAGE, output->width * 4, 1);
					auto CmykToRgb = [](uint8_t* rgb, uint8_t* cmyk)
					{
						uint32_t k1 = 255 - cmyk[3];
						uint32_t k2 = cmyk[3];
						for (int i = 0; i < 3; ++i)
						{
							uint32_t c = k1 + k2 * (255 - cmyk[i]) / 255;
							rgb[i] = (c > 255) ? 0 : (255 - c);
						}
					};
					while (cinfo.output_scanline < cinfo.output_height)
					{
						jpeg_read_scanlines(&cinfo, buffer, 1);
						uint8_t* input = (uint8_t*)buffer[0];
						for (size_t i = 0; i < cinfo.output_width; ++i)
						{
							CmykToRgb(data, input);
							data += 3;
							input += 4;
						}
					}
				}
			}
			else
			{
				DKLogE("[DKImage::Create] Error: Out of memory!");				
			}
			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			return output;
		}
		break;
	case FormatBMP:
		if (s > sizeof(BMPFileHeader) + sizeof(BMPCoreHeader))
		{
			auto CheckOverflow = [s](size_t pos)->bool
			{
				if (s < pos)
				{
					DKLogE("[DKImage::Create] Error: BMP data overflow!");
					return false;
				}
				return true;
			};

			const uint8_t* data = reinterpret_cast<const uint8_t*>(p);
			size_t pos = 0;
			BMPFileHeader fileHeader = *reinterpret_cast<const BMPFileHeader*>(data);
			fileHeader.size = DKLittleEndianToSystem(fileHeader.size);
			fileHeader.offBits = DKLittleEndianToSystem(fileHeader.offBits);
			if (!CheckOverflow(fileHeader.size))
				return NULL;
			if (!CheckOverflow(fileHeader.offBits))
				return NULL;

			pos += sizeof(BMPFileHeader);
			size_t headerSize = DKLittleEndianToSystem(reinterpret_cast<const BMPCoreHeader*>(&data[pos])->size);

			if (!CheckOverflow(pos + headerSize))
				return NULL;

			size_t colorTableEntrySize;
			BMPInfoHeader info = {};
			if (headerSize >= sizeof(BMPInfoHeader))
			{
				info = *reinterpret_cast<const BMPInfoHeader*>(&data[pos]);
				info.size = DKLittleEndianToSystem(info.size);
				info.width = DKLittleEndianToSystem(info.width);
				info.height = DKLittleEndianToSystem(info.height);
				info.planes = DKLittleEndianToSystem(info.planes);
				info.bitCount = DKLittleEndianToSystem(info.bitCount);
				info.compression = DKLittleEndianToSystem(info.compression);
				info.sizeImage = DKLittleEndianToSystem(info.sizeImage);
				info.xPelsPerMeter = DKLittleEndianToSystem(info.xPelsPerMeter);
				info.yPelsPerMeter = DKLittleEndianToSystem(info.yPelsPerMeter);
				info.clrUsed = DKLittleEndianToSystem(info.clrUsed);
				info.clrImportant = DKLittleEndianToSystem(info.clrImportant);
				colorTableEntrySize = 4; // RGBA
			}
			else if (headerSize >= sizeof(BMPCoreHeader))
			{
				const BMPCoreHeader& core = *reinterpret_cast<const BMPCoreHeader*>(&data[pos]);
				info.size = DKLittleEndianToSystem(core.size);
				info.width = DKLittleEndianToSystem(core.width);
				info.height = DKLittleEndianToSystem(core.height);
				info.planes = DKLittleEndianToSystem(core.planes);
				info.bitCount = DKLittleEndianToSystem(core.bitCount);
				info.compression = BMPCompressionRGB;
				info.sizeImage = 0;
				info.xPelsPerMeter = BMP_DEFAULT_PPM;
				info.yPelsPerMeter = BMP_DEFAULT_PPM;
				info.clrUsed = 0;
				info.clrImportant = 0;
				colorTableEntrySize = 3; // old-style
			}
			else
			{
				DKLogE("[DKImage::Create] Error: Unsupported bitmap format.");
				return NULL;
			}
			if (info.bitCount != 1 && info.bitCount != 4 && info.bitCount != 8 &&
				info.bitCount != 16 && info.bitCount != 24 && info.bitCount != 32)
			{
				DKLogE("[DKImage::Create] Error: Unsupported bitmap format.");
				return NULL;
			}
			if ((info.compression == BMPCompressionRLE4 && info.bitCount != 4) ||
				(info.compression == BMPCompressionRLE8 && info.bitCount != 8) ||
				(info.compression == BMPCompressionBITFIELDS && (info.bitCount != 16 && info.bitCount != 32)))
			{
				DKLogE("[DKImage::Create] Error: Invalid BMP data format.");
				return NULL;
			}
			bool topDown = info.height < 0;
			if (topDown)
				info.height = -info.height;

			if (info.width <= 0 || info.height <= 0)
			{
				DKLogE("[DKImage::Create] Error: Invalid BMP data format.");
				return NULL;
			}
			pos += info.size; // set position to color-table map (if available)

			if ((info.compression == BMPCompressionRLE8) || (info.compression == BMPCompressionRLE4))
			{
				DKObject<DKImage> image = Create(info.width, info.height, RGB8, nullptr);
				if (image == nullptr)
				{
					DKLogE("[DKImage::Create] Error: Out of memory!");
					return NULL;
				}
				const uint8_t* colorTableEntries = &data[pos];
				uint8_t* output = reinterpret_cast<uint8_t*>(image->data);
				// set background color to first color-table entry
				for (size_t i = 0, n = info.width * info.height * 3; i < n; ++i)
					output[i] = colorTableEntries[2 - (i % 3)];

				auto SetPixelAtPosition = [&](int32_t x, int32_t y, uint8_t index)
				{
					if (x < info.width && y < info.height)
					{
						if (!topDown)
							y = info.height - y - 1;

						DKASSERT_DEBUG(index < (1 << info.bitCount));
						uint8_t* data = &output[(y * info.width + x) * 3];
						const uint8_t* cm = &colorTableEntries[uint32_t(4) * uint32_t(index)];
						data[0] = cm[2];
						data[1] = cm[1];
						data[2] = cm[0];
					}
				};
				// set position to bitmap data
				pos = fileHeader.offBits;
				int32_t x = 0;
				int32_t y = 0;
				while ((pos + 1) < s && y < info.height)
				{
					uint32_t first = data[pos++];
					uint32_t second = data[pos++];
					if (first == 0)
					{
						switch (second)
						{
						case 0:		// end of line
							x = 0;
							y++;
							break;
						case 1:		// end of bitmap
							y = info.height;
							break;
						case 2:		// move position 
							if (pos + 1 < s)
							{
								uint8_t deltaX = data[pos++];
								uint8_t deltaY = data[pos++];
								x += deltaX / (8 / info.bitCount);
								y += deltaY;
							}
							break;
						default:	// absolute mode.
							if (info.compression == BMPCompressionRLE8)
							{
								for (uint32_t i = 0; i < second && pos < s && x < info.width; ++i)
								{
									uint8_t index = data[pos++];
									SetPixelAtPosition(x++, y, index);
								}
								if (second & 1)
									pos++;
							}
							else
							{
								uint8_t nibble[2];
								uint32_t bytesRead = 0;
								for (uint32_t i = 0; i < second && pos < s && x < info.width; ++i)
								{
									if (!(i % 2))
									{
										bytesRead++;
										uint8_t index = data[pos++];
										nibble[0] = (index >> 4) & 0xf;
										nibble[1] = index & 0xf;
									}
									SetPixelAtPosition(x++, y, nibble[i % 2]);
								}
								if (bytesRead & 1)
									pos++;
							}
							break;
						}
					}
					else
					{
						if (info.compression == BMPCompressionRLE8)
						{
							while (first > 0 && x < info.width)
							{
								SetPixelAtPosition(x++, y, second);
								first--;
							}
						}
						else
						{
							while (first > 0 && x < info.width)
							{
								uint8_t h = (second >> 4) & 0xf;
								SetPixelAtPosition(x++, y, h);
								first--;
								if (first > 1)
								{
									uint8_t l = second & 0xf;
									SetPixelAtPosition(x++, y, l);
									first--;
								}
							}
						}
					}
				}
				return image;
			}
			else
			{
				uint32_t rowBytes = info.width * info.bitCount;
				if (rowBytes % 8)
					rowBytes = rowBytes / 8 + 1;
				else
					rowBytes = rowBytes / 8;
				// each row must be align of 4-bytes
				size_t rowBytesAligned = (rowBytes % 4) ? (rowBytes | 0x3) + 1 : rowBytes;

				size_t requiredBytes = rowBytesAligned * (info.height - 1) + rowBytes;

				if (!CheckOverflow(fileHeader.offBits + requiredBytes))
					return NULL;

				const uint8_t* bitmapData = &data[fileHeader.offBits];

				if (info.compression == BMPCompressionBITFIELDS)
				{
					uint32_t bitMask[3] = {
						DKLittleEndianToSystem(reinterpret_cast<const uint32_t*>(&data[pos])[0]),
						DKLittleEndianToSystem(reinterpret_cast<const uint32_t*>(&data[pos])[1]),
						DKLittleEndianToSystem(reinterpret_cast<const uint32_t*>(&data[pos])[2])
					};
					uint32_t bitShift[3] = { 0, 0, 0 };
					uint32_t numBits[3] = { 0, 0, 0 };

					for (int bit = 31; bit >= 0; --bit)
					{
						for (int i = 0; i < 3; ++i)
						{
							if (bitMask[i] & (1U << bit))
								bitShift[i] = bit;
						}
					}
					for (int i = 0; i < 3; ++i)
						bitMask[i] = bitMask[i] >> bitShift[i];

					for (int bit = 0; bit < 32; ++bit)
					{
						for (int i = 0; i < 3; ++i)
						{
							if (bitMask[i] & (1U << bit))
								numBits[i] = bit + 1;
						}
					}
					if (numBits[0] <= 8 && numBits[1] <= 8 && numBits[2] <= 8) // RGB8
					{
						uint32_t lshift[3] = { 8 - numBits[0], 8 - numBits[1], 8 - numBits[2] };

						DKObject<DKImage> image = Create(info.width, info.height, RGB8, nullptr);
						if (image == nullptr)
						{
							DKLogE("[DKImage::Create] Error: Out of memory!");
							return NULL;
						}

						uint8_t* output = reinterpret_cast<uint8_t*>(image->data);

						if (info.bitCount == 32)
						{
							auto SetRowPixels = [&](uint8_t*& output, const uint32_t* input, uint32_t width)
							{
								for (uint32_t x = 0; x < width; ++x)
								{
									uint32_t rgb = DKLittleEndianToSystem(*input);
									for (int i = 0; i < 3; ++i)
									{
										output[0] = ((rgb >> bitShift[i]) & bitMask[i]) << lshift[i];
										output++;
									}
									input++;
								}
							};
							if (topDown)
							{
								for (int32_t y = 0; y < info.height; ++y)
								{
									const uint32_t* row = reinterpret_cast<const uint32_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
							else
							{
								for (int32_t y = info.height - 1; y >= 0; --y)
								{
									const uint32_t* row = reinterpret_cast<const uint32_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
						}
						else // 16
						{
							auto SetRowPixels = [&](uint8_t*& output, const uint16_t* input, uint32_t width)
							{
								for (uint32_t x = 0; x < width; ++x)
								{
									uint16_t rgb = DKLittleEndianToSystem(*input);
									for (int i = 0; i < 3; ++i)
									{
										output[0] = ((rgb >> bitShift[i]) & bitMask[i]) << lshift[i];
										output++;
									}
									input++;
								}
							};
							if (topDown)
							{
								for (int32_t y = 0; y < info.height; ++y)
								{
									const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
							else
							{
								for (int32_t y = info.height - 1; y >= 0; --y)
								{
									const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
						}
						return image;
					}
					else // RGB32F
					{
						DKObject<DKImage> image = Create(info.width, info.height, RGB32F, nullptr);
						if (image == nullptr)
						{
							DKLogE("[DKImage::Create] Error: Out of memory!");
							return NULL;
						}

						float denum[3] = {
							static_cast<float>(bitMask[0]),
							static_cast<float>(bitMask[1]),
							static_cast<float>(bitMask[2])
						};

						float* output = reinterpret_cast<float*>(image->data);

						if (info.bitCount == 32)
						{
							auto SetRowPixels = [&](float*& output, const uint32_t* input, uint32_t width)
							{
								for (uint32_t x = 0; x < width; ++x)
								{
									uint32_t rgb = DKLittleEndianToSystem(*input);
									for (int i = 0; i < 3; ++i)
									{
										if (denum[i] != 0.0f)
											output[0] = static_cast<float>((rgb >> bitShift[i]) & bitMask[i]) / denum[i];
										else
											output[0] = 0.0f;
										output++;
									}
									input++;
								}
							};
							if (topDown)
							{
								for (int32_t y = 0; y < info.height; ++y)
								{
									const uint32_t* row = reinterpret_cast<const uint32_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
							else
							{
								for (int32_t y = info.height - 1; y >= 0; --y)
								{
									const uint32_t* row = reinterpret_cast<const uint32_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
						}
						else // 16
						{
							auto SetRowPixels = [&](float*& output, const uint16_t* input, uint32_t width)
							{
								for (uint32_t x = 0; x < width; ++x)
								{
									uint16_t rgb = DKLittleEndianToSystem(*input);
									for (int i = 0; i < 3; ++i)
									{
										if (denum[i] != 0.0f)
											output[0] = static_cast<float>((rgb >> bitShift[i]) & bitMask[i]) / denum[i];
										else
											output[0] = 0.0f;
										output++;
									}
									input++;
								}
							};
							if (topDown)
							{
								for (int32_t y = 0; y < info.height; ++y)
								{
									const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
							else
							{
								for (int32_t y = info.height - 1; y >= 0; --y)
								{
									const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
									SetRowPixels(output, row, info.width);
								}
							}
						}
						return image;
					}
				}
				else if (info.bitCount == 32 || info.bitCount == 24)
				{
					DKObject<DKImage> image;
					if (info.bitCount == 32)
						image = Create(info.width, info.height, RGBA8, nullptr);
					else
						image = Create(info.width, info.height, RGB8, nullptr);
					if (image == nullptr)
					{
						DKLogE("[DKImage::Create] Error: Out of memory!");
						return NULL;
					}
					int32_t bpp = info.bitCount / 8;
					const uint32_t colorIndices[] = { 2, 1, 0, 3 }; // BGRA -> RGBA

					auto SetRowPixels = [&](uint8_t*& output, const uint8_t* input, uint32_t width, uint8_t bpp)
					{
						for (uint32_t x = 0; x < width; ++x)
						{
							for (int32_t i = 0; i < bpp; ++i)
								output[i] = input[colorIndices[i]];
							output += bpp;
							input += bpp;
						}
					};

					uint8_t* output = reinterpret_cast<uint8_t*>(image->data);
					if (topDown)
					{
						for (int32_t y = 0; y < info.height; ++y)
						{
							const uint8_t* row = &bitmapData[rowBytesAligned * y];
							SetRowPixels(output, row, info.width, bpp);
						}
					}
					else
					{
						for (int32_t y = info.height - 1; y >= 0; --y)
						{
							const uint8_t* row = &bitmapData[rowBytesAligned * y];
							SetRowPixels(output, row, info.width, bpp);
						}
					}
					return image;
				}
				else if (info.bitCount == 16)
				{
					DKObject<DKImage> image = Create(info.width, info.height, RGB8, nullptr);
					if (image == nullptr)
					{
						DKLogE("[DKImage::Create] Error: Out of memory!");
						return NULL;
					}
					uint8_t* output = reinterpret_cast<uint8_t*>(image->data);
					auto SetRowPixels = [&](uint8_t*& output, const uint16_t* input, uint32_t width)
					{
						for (uint32_t x = 0; x < width; ++x)
						{
							uint16_t pixel = DKLittleEndianToSystem(*input);
							uint16_t r = (pixel & 0x7c00) >> 10;
							uint16_t g = (pixel & 0x03e0) >> 5;
							uint16_t b = (pixel & 0x001f);

							output[0] = static_cast<uint8_t>((r << 3) & 0xff);
							output[1] = static_cast<uint8_t>((g << 3) & 0xff);
							output[2] = static_cast<uint8_t>((b << 3) & 0xff);

							output += 3;
							input++;
						}
					};
					if (topDown)
					{
						for (int32_t y = 0; y < info.height; ++y)
						{
							const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
							SetRowPixels(output, row, info.width);
						}
					}
					else
					{
						for (int32_t y = info.height - 1; y >= 0; --y)
						{
							const uint16_t* row = reinterpret_cast<const uint16_t*>(&bitmapData[rowBytesAligned * y]);
							SetRowPixels(output, row, info.width);
						}
					}
					return image;
				}
				else // 1, 4, 8
				{
					DKObject<DKImage> image = Create(info.width, info.height, RGB8, nullptr);
					if (image == nullptr)
					{
						DKLogE("[DKImage::Create] Error: Out of memory!");
						return NULL;
					}
					auto SetPixelFromCMap = [&](uint8_t index, uint8_t* output)
					{
						const uint8_t* colorTableEntries = &data[pos];
						DKASSERT_DEBUG(index < (1 << info.bitCount));
						const uint8_t* c = &colorTableEntries[uint32_t(colorTableEntrySize) * uint32_t(index)];
						output[0] = c[2];
						output[1] = c[1];
						output[2] = c[0];
					};
					auto SetRowPixels = [&](uint8_t*& output, const uint8_t* input, size_t width, uint16_t bits, uint8_t mask)
					{
						int32_t x = 0;
						while (x < width)
						{
							uint8_t c = *input;
							for (int32_t bit = 0; bit < 8 && x < width; ++x)
							{
								bit += bits;
								uint8_t index = (c >> (8 - bit)) & mask;
								SetPixelFromCMap(index, output);
								output += 3; //RGB8
							}
							input++;
						}
						DKASSERT_DEBUG(x == width);
					};
					uint8_t* output = reinterpret_cast<uint8_t*>(image->data);
					uint8_t pixelMask = (1 << info.bitCount) - 1;

					if (topDown)
					{
						for (int32_t y = 0; y < info.height;++y)
						{
							const uint8_t* row = &bitmapData[rowBytesAligned * y];
							SetRowPixels(output, row, info.width, info.bitCount, pixelMask);
						}
					}
					else
					{
						for (int32_t y = info.height - 1; y >= 0; --y)
						{
							const uint8_t* row = &bitmapData[rowBytesAligned * y];
							SetRowPixels(output, row, info.width, info.bitCount, pixelMask);
						}
					}
					return image;
				}
			}
		}
		else
		{
			DKLogE("[DKImage::Create] Error: BMP data size is too small!");
		}
		break;
	}
	return NULL;
}

DKObject<DKData> DKImage::EncodeData(const DKString& str, DKOperationQueue* queue) const
{
	if (!IsValid())
	{
		DKLogE("[DKImage::EncodeData] Error: Image is not valid!");
		return NULL;
	}
	if (this->width & 0x80000000 || this->height & 0x80000000)
	{
		DKLogE("[DKImage::EncodeData] Error: Image is too large to encode");
		return NULL;
	}

	ImageFormat fmt;
	if (!IdentifyImageFormatFromString(str, fmt))
	{
		DKLogE("[DKImage::EncodeData] Error: Unable to identify format.");
		return NULL;
	}
	switch (fmt)
	{
	case FormatPNG:
		if (1)
		{
			png_uint_32 pngFormat;
			PixelFormat targetFormat = this->format;
			switch (this->format)
			{
			case R8:
				pngFormat = PNG_FORMAT_GRAY;
				break;
			case RG8:
				pngFormat = PNG_FORMAT_GA;
				break;
			case RGB8:
				pngFormat = PNG_FORMAT_RGB;
				break;
			case RGBA8:
				pngFormat = PNG_FORMAT_RGBA;
				break;
			case R16:
				pngFormat = PNG_FORMAT_LINEAR_Y;
				break;
			case RG16:
				pngFormat = PNG_FORMAT_LINEAR_Y_ALPHA;
				break;
			case RGB16:
				pngFormat = PNG_FORMAT_LINEAR_RGB;
				break;
			case RGBA16:
				pngFormat = PNG_FORMAT_LINEAR_RGB_ALPHA;
				break;
				/* below formats are not able to encode directly, resample required */
			case R32:
			case R32F:
				targetFormat = R8;
				break;
			case RG32:
			case RG32F:
				targetFormat = RG8;
				break;
			case RGB32:
			case RGB32F:
				targetFormat = RGB8;
				break;
			case RGBA32:
			case RGBA32F:
				targetFormat = RGBA8;
				break;
			default:
				DKLogE("[DKImage::EncodedData] Error: Invalid pixel format!");
				return NULL;
			}
			if (targetFormat != this->format) // resample required
			{
				DKObject<DKImage> resampledImage = this->Resample(this->width, this->height, targetFormat, Nearest, queue);
				if (resampledImage)
					return resampledImage->EncodeData(str, queue);
				DKLogE("[DKImage::EncodeData] Error: Unable to resample format.");
				return NULL;
			}
			png_image image = {};
			image.version = PNG_IMAGE_VERSION;
			image.width = this->width;
			image.height = this->height;
			image.format = pngFormat;

			png_alloc_size_t bufferSize = 0;
			if (png_image_write_get_memory_size(image, bufferSize, 0, this->data, 0, nullptr) && bufferSize > 0)
			{
				DKObject<DKBuffer> output = DKBuffer::Create(0, bufferSize);
				if (output)
				{
					DKDataWriter writer(output);
					bufferSize = writer.Length();
					if (png_image_write_to_memory(&image, writer.Bytes(), &bufferSize, 0, this->data, 0, nullptr))
					{
						return output.SafeCast<DKData>();
					}
				}
				else
				{
					DKLogE("[DKImage::EncodeData] Error: DKBuffer creation failure.");
					return NULL;
				}
			}
			DKLogE("[DKImage::EncodeData] Error: PNG write failed.");
		}
		break;
	case FormatJPEG:
		if (1)
		{
			PixelFormat targetFormat = this->format;
			switch (this->format)
			{
			case R8:				
			case R16:
			case R32:
			case R32F:
				targetFormat = R8;
				break;
			default:
				targetFormat = RGB8;
				break;
			}
			if (targetFormat != this->format) // resample required
			{
				DKObject<DKImage> resampledImage = this->Resample(this->width, this->height, targetFormat, Nearest, queue);
				if (resampledImage)
					return resampledImage->EncodeData(str, queue);
				DKLogE("[DKImage::EncodeData] Error: Unable to resample format.");
				return NULL;
			}
			struct JpegDestination
			{
				struct jpeg_destination_mgr pub;
				JOCTET* buffer;
				DKBufferStream stream;
			};

			jpeg_compress_struct cinfo = {};
			JpegErrorMgr err = {};
			JpegDestination dest;

			dest.pub.init_destination = [](j_compress_ptr cinfo)
			{
				JpegDestination* dest = reinterpret_cast<JpegDestination*>(cinfo->dest);
				dest->buffer = (JOCTET*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,
																   JPOOL_IMAGE,
																   JPEG_BUFFER_SIZE * sizeof(JOCTET));
				dest->pub.next_output_byte = dest->buffer;
				dest->pub.free_in_buffer = JPEG_BUFFER_SIZE;
			};
			dest.pub.empty_output_buffer = [](j_compress_ptr cinfo)->boolean
			{
				JpegDestination* dest = reinterpret_cast<JpegDestination*>(cinfo->dest);
				dest->stream.Write(dest->buffer, JPEG_BUFFER_SIZE);
				dest->pub.next_output_byte = dest->buffer;
				dest->pub.free_in_buffer = JPEG_BUFFER_SIZE;
				return TRUE;
			};
			dest.pub.term_destination = [](j_compress_ptr cinfo)
			{
				JpegDestination* dest = reinterpret_cast<JpegDestination*>(cinfo->dest);
				size_t length = JPEG_BUFFER_SIZE - dest->pub.free_in_buffer;
				if (length > 0)
					dest->stream.Write(dest->buffer, length);
			};

			JSAMPLE* buffer;
			int32_t rowStride;

			cinfo.err = jpeg_std_error(&err.pub);
			err.pub.error_exit = [](j_common_ptr cinfo)
			{
				JpegErrorMgr* err = (JpegErrorMgr*)cinfo->err;
				err->pub.format_message(cinfo, err->buffer);
				longjmp(err->setjmpBuffer, 1);
			};

			if (setjmp(err.setjmpBuffer))
			{
				DKLogE("[DKImage::EncodeData] JPEG Error: %s", err.buffer);
				jpeg_destroy_compress(&cinfo);
				return NULL;
			}

			jpeg_create_compress(&cinfo);

			cinfo.dest = (jpeg_destination_mgr*)&dest;

			cinfo.image_width = this->width;
			cinfo.image_height = this->height;
			if (format == RGB8)
			{
				cinfo.input_components = 3;
				cinfo.in_color_space = JCS_RGB;
			}
			else
			{
				cinfo.input_components = 1;
				cinfo.in_color_space = JCS_GRAYSCALE;
			}

			jpeg_set_defaults(&cinfo);
			//jpeg_set_quality(&cinfo, 75, true);

			jpeg_start_compress(&cinfo, TRUE);
			rowStride = this->width * 3; // bytesPerPixel = 3
			buffer = (JSAMPLE*)(this->data);
			JSAMPROW row[1];
			while (cinfo.next_scanline < cinfo.image_height)
			{
				row[0] = &buffer[cinfo.next_scanline * rowStride];
				jpeg_write_scanlines(&cinfo, row, 1);
			}
			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);
			return dest.stream.Buffer();
		}
		break;
	case FormatBMP:
		if (1)
		{
			PixelFormat targetFormat = this->format;
			switch (this->format)
			{
			case RGBA8:
			case RGBA16:
			case RGBA32:
			case RGBA32F:
				targetFormat = RGBA8;
				break;
			default:
				targetFormat = RGB8;
				break;
			}

			if (targetFormat != this->format)
			{
				DKObject<DKImage> resampledImage = this->Resample(this->width, this->height, RGB8, Nearest, queue);
				if (resampledImage)
					return resampledImage->EncodeData(str, queue);
				DKLogE("[DKImage::EncodeData] Error: Unable to resample format.");
				return NULL;
			}

			size_t bytesPerPixel = Private::BytesPerPixel(this->format);
			DKASSERT_DEBUG(bytesPerPixel == 3 || bytesPerPixel == 4);

			size_t rowBytes = bytesPerPixel * width;
			if (rowBytes % 4)
				rowBytes = (rowBytes | 0x3) + 1;
			size_t imageSize = rowBytes * this->height;
			size_t dataSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;

			DKObject<DKBuffer> output = DKBuffer::Create(0, dataSize);
			if (output && output->Length() >= dataSize)
			{
				uint16_t bitCount = uint16_t(bytesPerPixel) * 8;

				DKDataWriter writer(output);
				uint8_t* buffer = reinterpret_cast<uint8_t*>(writer.Bytes());
				BMPFileHeader* header = reinterpret_cast<BMPFileHeader*>(buffer);	buffer += sizeof(BMPFileHeader);
				BMPInfoHeader* info = reinterpret_cast<BMPInfoHeader*>(buffer);		buffer += sizeof(BMPInfoHeader);

				size_t fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
				header->b = 'B'; header->m = 'M';
				header->size = DKSystemToLittleEndian<uint32_t>(uint32_t(fileSize));
				header->reserved1 = 0;
				header->reserved2 = 0;
				header->offBits = DKSystemToLittleEndian<uint32_t>(sizeof(BMPFileHeader) + sizeof(BMPInfoHeader));

				info->size = DKSystemToLittleEndian<uint32_t>(sizeof(BMPInfoHeader));
				info->width = DKSystemToLittleEndian<int32_t>(static_cast<int32_t>(this->width));
				info->height = DKSystemToLittleEndian<int32_t>(static_cast<int32_t>(this->height));
				info->planes = DKSystemToLittleEndian<uint16_t>(1);
				info->bitCount = DKSystemToLittleEndian<uint16_t>(bitCount);
				info->compression = DKSystemToLittleEndian<uint32_t>(BMPCompressionRGB);
				info->sizeImage = 0;
				info->xPelsPerMeter = DKSystemToLittleEndian<int32_t>(BMP_DEFAULT_PPM);
				info->yPelsPerMeter = DKSystemToLittleEndian<int32_t>(BMP_DEFAULT_PPM);
				info->clrUsed = 0;
				info->clrImportant = 0;

				const uint32_t colorIndices[] = { 2, 1, 0, 3 }; // BGRA -> RGBA

				for (int32_t y = height -1; y >= 0; --y)
				{
					const uint8_t* pixelData = &reinterpret_cast<const uint8_t*>(data)[width * y * bytesPerPixel];
					size_t bytesPerRow = 0;
					for (uint32_t x = 0; x < width; ++x)
					{
						for (uint32_t i = 0; i < bytesPerPixel; ++i)
							buffer[i] = pixelData[colorIndices[i]]; // BGR(A)

						buffer += bytesPerPixel;
						pixelData += bytesPerPixel;
						bytesPerRow += bytesPerPixel;
					}
					while (bytesPerRow < rowBytes)
					{
						buffer[0] = 0;
						buffer++;
						bytesPerRow++;
					}
				}
				return output.SafeCast<DKData>();
			}
			else
			{
				DKLogE("[DKImage::EncodeData] Error: Out of memory!");
			}
		}
		break;
	}
	DKLogE("[DKImage::EncodeData] Error: Internal error.");
	return NULL;
}

DKObject<DKImage> DKImage::Resample(uint32_t w, uint32_t h, PixelFormat f, Interpolation intp, DKOperationQueue* queue) const
{
	if (!IsValid())
	{
		DKLogE("[DKImage::Resample] Error: Image is not valid!");
		return NULL;
	}
	if (w > 0 && h > 0 && IsFormatValid(f))
	{
		DKObject<DKImage> output = DKOBJECT_NEW DKImage();
		output->width = w;
		output->height = h;
		output->format = f;

		if (w == this->width && h == this->height && f == this->format)
		{
			size_t s = Private::BytesPerPixel(f);
			size_t dataSize = s * w * h;
			output->data = DKMalloc(dataSize);
			if (output->data)
			{
				memcpy(output->data, this->data, dataSize);
			}
			else
			{
				DKLogE("[DKImage::Resample] Error: Out of memory!");
				output = NULL;
			}
		}
		else
		{
			//TODO: Resample image format!
		}
		return NULL;
	}
	else
	{
		DKLogE("[DKImage::Resample] Error: Invalid arguments.");
	}
	return NULL;
}

DKObject<DKSerializer> DKImage::Serializer()
{
	class LocalSerializer : public DKSerializer
	{
		size_t ComponentSize(PixelFormat pf)
		{
			switch (pf)
			{
			case R8:
			case RG8:
			case RGB8:
			case RGBA8:
				return 1;
			case R16:
			case RG16:
			case RGB16:
			case RGBA16:
				return 2;
			case R32:
			case RG32:
			case RGB32:
			case RGBA32:
			case R32F:
			case RG32F:
			case RGB32F:
			case RGBA32F:
				return 4;
			}
			return 1;
		}
		bool CheckPixelFormat(PixelFormat pf)
		{
			switch (pf)
			{
			case R8:
			case RG8:
			case RGB8:
			case RGBA8:
			case R16:
			case RG16:
			case RGB16:
			case RGBA16:
			case R32:
			case RG32:
			case RGB32:
			case RGBA32:
			case R32F:
			case RG32F:
			case RGB32F:
			case RGBA32F:
					return true;
			}
			return false;
		}
	public:
		DKSerializer* Init(DKImage* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKImage");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);
			this->Bind(L"width",
					   DKFunction([this](DKVariant& v) {v.SetInteger(this->target->width);}),
					   DKFunction([this](DKVariant& v) {this->target->width = v.Integer();}),
					   DKFunction([](const DKVariant& v) { return v.ValueType() == DKVariant::TypeInteger;}),
					   NULL);
			this->Bind(L"height",
					   DKFunction([this](DKVariant& v) {v.SetInteger(this->target->height);}),
					   DKFunction([this](DKVariant& v) {this->target->height = v.Integer();}),
					   DKFunction([](const DKVariant& v) { return v.ValueType() == DKVariant::TypeInteger;}),
					   NULL);
			this->Bind(L"pixelFormat",
					   DKFunction([this](DKVariant& v) {v.SetInteger(this->target->format);}),
					   DKFunction([this](DKVariant& v) {this->target->format = (PixelFormat)v.Integer();}),
					   DKFunction([this](const DKVariant& v)
					   {
							return v.ValueType() == DKVariant::TypeInteger &&
							CheckPixelFormat((PixelFormat)v.Integer());
					   }),
					   NULL);
			this->Bind("data",
					   DKFunction([this](DKVariant& v)
					   {
							size_t dataLength = target->width * target->height * target->BytesPerPixel();
							DKVariant::VStructuredData& st = v.SetValueType(DKVariant::TypeStructData).StructuredData();
							st.data = DKData::StaticData(this->target->data, dataLength);
							st.elementSize = ComponentSize(this->target->format);
							st.layout.Add((DKVariant::StructElem)ComponentSize(this->target->format));
					   }),
					   DKFunction([this](DKVariant& v)
					   {
						   if (target->data)
							   DKFree(target->data);
						   target->data = NULL;

						   if (v.ValueType() == DKVariant::TypeData)
						   {
							   DKVariant::VData& data = v.Data();
							   DKDataReader reader(&data);
							   target->data = DKMalloc(reader.Length());
							   if (target->data)
								   memcpy(target->data, reader.Bytes(), reader.Length());
						   }
						   else if (v.ValueType() == DKVariant::TypeStructData)
						   {
							   DKVariant::VStructuredData& st = v.StructuredData();
							   DKDataReader reader(st.data);
							   target->data = DKMalloc(reader.Length());
							   if (target->data)
								   memcpy(target->data, reader.Bytes(), reader.Length());
						   }
					   }),
					   DKFunction([this](const DKVariant& v)
					   {
							return v.ValueType() == DKVariant::TypeData ||
								   v.ValueType() == DKVariant::TypeStructData;
		       		   }),
					   NULL);
			return this;
		}
	private:
		DKObject<DKImage> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
