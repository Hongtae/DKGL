//
//  File: DKCompressor.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include <ctype.h>
#include <wctype.h>

#include "../Libs/zlib/zlib.h"

#include "../Libs/lz4/lib/lz4.h"
#include "../Libs/lz4/lib/lz4hc.h"
#include "../Libs/lz4/lib/lz4frame.h"
#include "../Libs/lz4/lib/xxhash.h"

#include "DKCompressor.h"
#include "DKEndianness.h"
#include "DKLog.h"

#define COMPRESSION_CHUNK_SIZE 0x40000

using namespace DKFoundation;

DKCompressor::DKCompressor(Method m)
	: method(m)
{
}

DKCompressor::~DKCompressor(void)
{
}

bool DKCompressor::Compress(DKStream* input, DKStream* output) const
{
	if (input == NULL || input->IsReadable() == false)
		return false;
	if (output == NULL || output->IsWritable() == false)
		return false;

	if (method == Deflate)
	{
		int err = Z_OK;
		z_stream stream = {};
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;

		uInt inputBufferSize = (uInt)COMPRESSION_CHUNK_SIZE;
		uInt outputBufferSize = (uInt)COMPRESSION_CHUNK_SIZE;
		void* inputBuffer = DKMalloc(inputBufferSize);
		void* outputBuffer = DKMalloc(outputBufferSize);

		if (inputBuffer == NULL || outputBuffer == NULL)
		{
			DKLog("DKCompressor Error: Out of memory!");
			if (inputBuffer)
				DKFree(inputBuffer);
			if (outputBuffer)
				DKFree(outputBuffer);
			return false;
		}

		int compressLevel = 9;	// Z_DEFAULT_COMPRESSION is 6
		err = deflateInit(&stream, compressLevel);
		if (err == Z_OK)
		{
			int flush = Z_NO_FLUSH;
			while (err == Z_OK)
			{
				if (stream.avail_in == 0)
				{
					size_t inputSize = input->Read(inputBuffer, inputBufferSize);
					if (inputSize == DKStream::PositionError)
					{
						DKLog("DKCompressor Error: Input stream error!");
						err = Z_STREAM_ERROR;
						break;
					}
					else if (inputSize == 0)
					{
						err = Z_STREAM_END;
						flush = Z_FINISH;
					}

					stream.avail_in = (uInt)inputSize;
					stream.next_in = (Bytef*)inputBuffer;
				}

				stream.avail_out = outputBufferSize;
				stream.next_out = (Bytef*)outputBuffer;
				err = deflate(&stream, flush);
				DKASSERT_DEBUG(err != Z_STREAM_ERROR);

				size_t write = outputBufferSize - stream.avail_out;
				if (output->Write(outputBuffer, write) != write)
				{
					DKLog("DKCompressor Error: Output stream error!");
					err = Z_STREAM_ERROR;
					break;
				}
			}
			deflateEnd(&stream);
		}

		DKFree(inputBuffer);
		DKFree(outputBuffer);

		return err == Z_STREAM_END;
	}
	else if (method == LZ4 || method == LZ4HC)
	{
		LZ4F_compressionContext_t ctx;
		LZ4F_errorCode_t err = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);

		if (!LZ4F_isError(err))
		{
			LZ4F_preferences_t prefs = {};
			prefs.autoFlush = 1;
			prefs.compressionLevel = method == LZ4 ? 0 : 9;	// 0 for LZ4 fast, 9 for LZ4HC
			prefs.frameInfo.blockMode = LZ4F_blockLinked;	// for better compression ratio.
			prefs.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled; // to detect data corruption.
			prefs.frameInfo.blockSizeID = LZ4F_max4MB;

			size_t inputBufferSize = size_t(1) << (8 + (2 * prefs.frameInfo.blockSizeID));
			size_t outputBufferSize = LZ4F_compressFrameBound(inputBufferSize, &prefs);;
			void* inputBuffer = DKMalloc(inputBufferSize);
			void* outputBuffer = DKMalloc(outputBufferSize);

			if (inputBuffer == NULL || outputBuffer == NULL)
			{
				DKLog("DKCompressor Error: Out of memory!");
				if (inputBuffer)
					DKFree(inputBuffer);
				if (outputBuffer)
					DKFree(outputBuffer);
				return false;
			}

			bool result = false;

			size_t inputSize = input->Read(inputBuffer, inputBufferSize);
			if (inputSize != DKStream::PositionError)
			{
				// generate header
				size_t headerSize = LZ4F_compressBegin(ctx, outputBuffer, outputBufferSize, &prefs);
				if (!LZ4F_isError(headerSize))
				{
					// write header
					if (output->Write(outputBuffer, headerSize) == headerSize)
					{
						result = true;
						// compress block
						while (inputSize > 0)
						{
							size_t outputSize = LZ4F_compressUpdate(ctx, outputBuffer, outputBufferSize, inputBuffer, inputSize, NULL);
							if (LZ4F_isError(outputSize))
							{
								DKLog("DKCompressor Error: LZ4 Encoding error: %s", LZ4F_getErrorName(outputSize));
								result = false;
								break;
							}
							if (output->Write(outputBuffer, outputSize) != outputSize)
							{
								DKLog("DKCompressor Error: Output stream error!");
								result = false;
								break;
							}
							inputSize = input->Read(inputBuffer, inputBufferSize);
							if (inputSize == DKStream::PositionError)
							{
								DKLog("DKCompressor Error: Input stream error!");
								result = false;
								break;
							}
						}
						if (result)
						{
							// generate footer
							headerSize = LZ4F_compressEnd(ctx, outputBuffer, outputBufferSize, NULL);
							if (!LZ4F_isError(headerSize))
							{
								// write footer
								result = output->Write(outputBuffer, headerSize) == headerSize;
								if (!result)
									DKLog("DKCompressor Error: Output stream error!");
							}
							else
							{
								result = false;
							}
						}
					}
				}
				else
				{
					DKLog("DKCompressor Error: LZ4 Encoder error: %s", LZ4F_getErrorName(headerSize));
				}
			}
			else
			{
				DKLog("DKCompressor Error: Input stream error!");
			}

			DKFree(inputBuffer);
			DKFree(outputBuffer);

			err = LZ4F_freeCompressionContext(ctx);
			DKASSERT_DEBUG(!LZ4F_isError(err));
			return result;
		}
		else
		{
			DKLog("DKCompressor Error: LZ4 Encoder error: %s", LZ4F_getErrorName(err));
		}
	}
	return false;
}

bool DKCompressor::Decompress(DKStream* input, DKStream* output)
{
	if (input == NULL || input->IsReadable() == false)
		return false;
	if (output == NULL || output->IsWritable() == false)
		return false;

	bool result = false;
	size_t inputBufferSize = (size_t)COMPRESSION_CHUNK_SIZE;
	void* inputBuffer = DKMalloc(inputBufferSize);
	size_t outputBufferSize = (size_t)COMPRESSION_CHUNK_SIZE;
	void* outputBuffer = DKMalloc(inputBufferSize);

	if (inputBuffer == NULL || outputBuffer == NULL)
	{
		DKLog("DKCompressor Error: Out of memory!");
		if (inputBuffer)
			DKFree(inputBuffer);
		if (outputBuffer)
			DKFree(outputBuffer);
		return false;
	}

	size_t inputSize = input->Read(inputBuffer, Min(inputBufferSize, 4096));
	Method method;
	if (DetectMethod(inputBuffer, inputSize, method))
	{
		if (method == Deflate)
		{
			int err = Z_OK;
			z_stream stream = {};
			stream.zalloc = Z_NULL;
			stream.zfree = Z_NULL;
			stream.opaque = Z_NULL;
			stream.avail_in = 0;
			stream.next_in = Z_NULL;
			err = inflateInit(&stream);
			if (err == Z_OK)
			{
				stream.avail_in = (uInt)inputSize;
				stream.next_in = (Bytef*)inputBuffer;

				while (err == Z_OK)
				{
					if (stream.avail_in == 0)
					{
						inputSize = input->Read(inputBuffer, inputBufferSize);
						if (inputSize == DKStream::PositionError)
						{
							DKLog("DKCompressor Error: Input stream error!");
							err = Z_STREAM_ERROR;
							break;
						}
						else if (inputSize == 0)
						{
							break;
						}
						stream.avail_in = (uInt)inputSize;
						stream.next_in = (Bytef*)inputBuffer;
					}

					stream.avail_out = (uInt)outputBufferSize;
					stream.next_out = (Bytef*)outputBuffer;
					err = inflate(&stream, Z_NO_FLUSH);
					DKASSERT_DEBUG(err != Z_STREAM_ERROR);

					if (err == Z_NEED_DICT || err == Z_DATA_ERROR || err == Z_MEM_ERROR)
						break;

					size_t write = outputBufferSize - stream.avail_out;
					if (write > 0)
					{
						if (output->Write(outputBuffer, write) != write)
						{
							DKLog("DKCompressor Error: Output stream error!");
							err = Z_STREAM_ERROR;
							break;
						}
					}
				}
				inflateEnd(&stream);
			}
			else
			{
				DKLog("DKCompressor Error: inflate error: %s", zError(err));
			}
			result = err == Z_STREAM_END;
		}
		else if (method == LZ4 || method == LZ4HC)
		{
			LZ4F_decompressionContext_t ctx;
			LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
			if (!LZ4F_isError(err))
			{
				const uint32_t lz4_Header = DKSystemToLittleEndian(0x184D2204U);
				const uint32_t lz4_SkipHeader = DKSystemToLittleEndian(0x184D2A50U);

				size_t processed = 0;
				size_t inSize, outSize;
				uint8_t* const inData = reinterpret_cast<uint8_t*>(inputBuffer);
				bool decodeError = false;
				LZ4F_errorCode_t nextToLoad;

				while (!decodeError)
				{
					if (inputSize == 0)
					{
						inputSize = input->Read(inputBuffer, inputBufferSize);
						if (inputSize == DKStream::PositionError)
						{
							DKLog("DKCompressor Error: Input stream error!");
							decodeError = true;
							break;
						}
						else if (inputSize == 0) // end steam
							break;
					}
					uint32_t header = reinterpret_cast<const uint32_t*>(&inData[processed])[0];
					if (header == lz4_Header)
					{
						do {
							while (processed < inputSize)
							{
								inSize = inputSize - processed;
								outSize = outputBufferSize;
								nextToLoad = LZ4F_decompress(ctx, outputBuffer, &outSize, &inData[processed], &inSize, NULL);
								if (LZ4F_isError(nextToLoad))
								{
									DKLog("Decompress Error: Lz4 Header Error: %s\n", LZ4F_getErrorName(nextToLoad));
									decodeError = true;
									nextToLoad = 0;
									break;
								}
								processed += inSize;
								if (outSize > 0)
								{
									if (output->Write(outputBuffer, outSize) != outSize)
									{
										DKLog("DKCompressor Error: Output stream error!");
										decodeError = true;
										nextToLoad = 0;
										break;
									}
								}
							}
							inputSize = 0;
							processed = 0;
							if (nextToLoad)
							{
								inputSize = input->Read(inputBuffer, Min(nextToLoad, inputBufferSize));
								if (inputSize == DKStream::PositionError)
								{
									DKLog("DKCompressor Error: Input stream error!");
									decodeError = true;
									break;
								}
							}
						} while (nextToLoad);
					}
					else if ((header & 0xfffffff0U) == lz4_SkipHeader)
					{
						while (inputSize < 8) // header + skip-length = 8
						{
							size_t n = input->Read(&inData[inputSize], 8 - inputSize);
							if (n == DKStream::PositionError)
							{
								DKLog("DKCompressor Error: Input stream error!");
								decodeError = true;
								break;
							}
							else if (n == 0) // end stream? 
							{
								DKLog("DKCompressor Error: Lz4 input stream ended before processing skip frame!\n");
								decodeError = true;
								break;
							}
							inputSize += n;
						}
						if (inputSize >= 8)
						{
							uint32_t bytesToSkip = reinterpret_cast<const uint32_t*>(&inData[processed])[1];
							bytesToSkip = DKLittleEndianToSystem(bytesToSkip);
							size_t remains = inputSize - processed;
							if (bytesToSkip > remains)
							{
								size_t offset = bytesToSkip - remains;
								if (input->SetCurrentPosition(input->CurrentPosition() + offset) == DKStream::PositionError)
								{
									DKLog("DKCompressor Error: Lz4 input stream cannot process skip frame!\n");
									decodeError = true;
									break;
								}
								inputSize = 0;
							}
							else
								processed += bytesToSkip;
						}
					}
					else
					{
						DKLog("Decompress Error: Lz4 stream followed by unrecognized data.\n");
						decodeError = true;
						break;
					}
				}

				err = LZ4F_freeDecompressionContext(ctx);
				DKASSERT_DEBUG(!LZ4F_isError(err));
				result = !decodeError;
			}
			else
			{
				DKLog("DKCompressor Error: LZ4 Decoder error: %s", LZ4F_getErrorName(err));
			}
		}
	}

	DKFree(inputBuffer);
	DKFree(outputBuffer);

	return result;
}

bool DKCompressor::DetectMethod(void* p, size_t n, Method& m)
{
	if (p && n >= 4)
	{
		const uint32_t lz4_Header = DKSystemToLittleEndian(0x184D2204U);
		const uint32_t lz4_SkipHeader = DKSystemToLittleEndian(0x184D2A50U);

		if (reinterpret_cast<const uint32_t*>(p)[0] == lz4_Header ||
			(reinterpret_cast<const uint32_t*>(p)[0] & 0xfffffff0U) == lz4_SkipHeader)
		{
			m = LZ4;
			return true;
		}
	}
	if (p && n >= 1)
	{
		if (reinterpret_cast<const char*>(p)[0] == 0x78)
		{
			m = Deflate;
			return true;
		}
	}
	return false;
}
