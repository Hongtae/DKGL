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
            Deflate,   	///< good compression ratio.
            Deflate9,   ///< deflate level 9 (best ratio)
            Zstd,       ///< better ratio, faster than Deflate.
            ZstdMax,
            LZ4,		///< fast compression, fast decompression.
            LZ4HC,		///< fast decompression, better compression ratio than LZ4.

            Default = Zstd,
            BestRatio = ZstdMax,
            Fastest = LZ4,
		};
		DKCompressor(Method);
		~DKCompressor();

		bool Compress(DKStream* input, DKStream* output) const;
		static bool Decompress(DKStream* input, DKStream* output);

	private:
		Method method;
	};
}
