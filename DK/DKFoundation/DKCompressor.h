//
//  File: DKCompressor.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKStream.h"
#include "DKFunction.h"

namespace DKFoundation
{
	/** @brief
	 A compression utility class, supports ZLib, Zstd, LZ4 compression.
	 */
	class DKCompressor
	{
	public:
        enum Method
        {
            Zlib,   	///< zlib deflate with level 5. Good compatibility.
            Zstd,       ///< Zstandard with level 3. Fast and good compression ratio. Good balance.
            ZstdMax,    ///< Zstandard compression level 19. fast decompression.
            LZ4,		///< fast compression, fast decompression.
            LZ4HC,		///< fast decompression, better compression ratio than LZ4.
            Lzma,		///< LZMA default (level 5)
            LzmaFast,	///< LZMA fastest (level 0)
            LzmaUltra,	///< LZMA ultra (level 9)

            Default = Zstd,
            BestRatio = LzmaUltra,
            Fastest = LZ4,

            Automatic,	///< Default method for compression, Auto-detected method for decompression.
        };
		DKCompressor(Method m = Automatic);
		~DKCompressor();

		bool Compress(DKStream* input, DKStream* output) const;
		bool Decompress(DKStream* input, DKStream* output) const;

		const Method method;
	};
}
