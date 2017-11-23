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
	class DKCompressor
	{
	public:
		enum Method {
			Deflate,	///< good compression ratio.
			LZ4,		///< fast compression, fast decompression.
			LZ4HC,		///< fast decompression, better compression ratio than LZ4.
		};
		DKCompressor(Method);
		~DKCompressor(void);

		bool Compress(DKStream* input, DKStream* output) const;
		static bool Decompress(DKStream* input, DKStream* output);

	private:
		static bool DetectMethod(void* p, size_t, Method&);
		Method method;
	};
}
