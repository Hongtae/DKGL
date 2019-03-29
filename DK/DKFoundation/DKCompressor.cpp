//
//  File: DKCompressor.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include <ctype.h>
#include <wctype.h>

#include "../Libs/zlib/zlib.h"

#include "../Libs/zstd/lib/zstd.h"

#include "../Libs/lz4/lib/lz4.h"
#include "../Libs/lz4/lib/lz4hc.h"
#include "../Libs/lz4/lib/lz4frame.h"
#include "../Libs/lz4/lib/xxhash.h"

#include "DKCompressor.h"
#include "DKEndianness.h"
#include "DKLog.h"

#define COMPRESSION_CHUNK_SIZE 0x40000

namespace DKFoundation::Private
{
    struct CompressorBuffer
    {
        void* buffer;
        size_t bufferSize;
        CompressorBuffer(size_t length)
            : bufferSize(0)
        {
            buffer = DKMalloc(length);
            if (buffer)
                bufferSize = length;
        }
        ~CompressorBuffer()
        {
            if (buffer)
                DKFree(buffer);
        }
    };

    static bool CompressDeflate(DKStream* input, DKStream* output, int level)
    {
        CompressorBuffer inputBuffer(COMPRESSION_CHUNK_SIZE);
        CompressorBuffer outputBuffer(COMPRESSION_CHUNK_SIZE);

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

        int err = Z_OK;
        z_stream stream = {};
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        int compressLevel = level;	// Z_DEFAULT_COMPRESSION is 6
        err = deflateInit(&stream, compressLevel);
        if (err == Z_OK)
        {
            int flush = Z_NO_FLUSH;
            while (err == Z_OK)
            {
                if (stream.avail_in == 0)
                {
                    size_t inputSize = input->Read(inputBuffer.buffer, inputBuffer.bufferSize);
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
                    stream.next_in = (Bytef*)inputBuffer.buffer;
                }

                stream.avail_out = (uInt)outputBuffer.bufferSize;
                stream.next_out = (Bytef*)outputBuffer.buffer;
                err = deflate(&stream, flush);
                DKASSERT_DEBUG(err != Z_STREAM_ERROR);

                size_t write = outputBuffer.bufferSize - stream.avail_out;
                if (output->Write(outputBuffer.buffer, write) != write)
                {
                    DKLog("DKCompressor Error: Output stream error!");
                    err = Z_STREAM_ERROR;
                    break;
                }
            }
            deflateEnd(&stream);
        }
        return err == Z_STREAM_END;
    }

    static bool DecompressDeflate(DKStream* input, DKStream* output)
    {
        CompressorBuffer inputBuffer(COMPRESSION_CHUNK_SIZE);
        CompressorBuffer outputBuffer(COMPRESSION_CHUNK_SIZE);

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

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
            while (err == Z_OK)
            {
                if (stream.avail_in == 0)
                {
                    size_t inputSize = input->Read(inputBuffer.buffer, inputBuffer.bufferSize);
                    if (inputSize == DKStream::PositionError)
                    {
                        DKLogE("DKCompressor Error: Input stream error!");
                        err = Z_STREAM_ERROR;
                        break;
                    }
                    else if (inputSize == 0)
                    {
                        break;
                    }
                    stream.avail_in = (uInt)inputSize;
                    stream.next_in = (Bytef*)inputBuffer.buffer;
                }

                stream.avail_out = (uInt)outputBuffer.bufferSize;
                stream.next_out = (Bytef*)outputBuffer.buffer;
                err = inflate(&stream, Z_NO_FLUSH);
                DKASSERT_DEBUG(err != Z_STREAM_ERROR);

                if (err == Z_NEED_DICT || err == Z_DATA_ERROR || err == Z_MEM_ERROR)
                    break;

                size_t write = outputBuffer.bufferSize - stream.avail_out;
                if (write > 0)
                {
                    if (output->Write(outputBuffer.buffer, write) != write)
                    {
                        DKLogE("DKCompressor Error: Output stream error!");
                        err = Z_STREAM_ERROR;
                        break;
                    }
                }
            }
            inflateEnd(&stream);

            return err == Z_STREAM_END;
        }
        else
        {
            DKLogE("DKCompressor Error: inflate error: %s", zError(err));
        }
        return false;
    }

    static bool CompressZstd(DKStream* input, DKStream* output, int level)
    {
        CompressorBuffer inputBuffer(ZSTD_CStreamInSize());
        CompressorBuffer outputBuffer(ZSTD_CStreamOutSize());

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

        /*
        Zstd requests a large amount of memory allocation.
        So we do not need to use DKMalloc.
        */
#if 0   
        ZSTD_customMem customMem = {
            [](void* opaque, size_t size)->void* {return DKMalloc(size); }, //ZSTD_allocFunction
            [](void* opaque, void* address) { DKFree(address); }, //ZSTD_freeFunction
            nullptr //opaque
        };
        ZSTD_CStream* const cstream = ZSTD_createCStream_advanced(customMem);
#else
        ZSTD_CStream* const cstream = ZSTD_createCStream();
#endif
        if (cstream)
        {
            bool result = false;
            size_t const initResult = ZSTD_initCStream(cstream, level);
            if (ZSTD_isError(initResult))
            {
                DKLogE("DKCompressor::Compress error: ZSTD_initCStream failed: %s",
                       ZSTD_getErrorName(initResult));
            }
            else
            {
                result = true;
                size_t toRead = inputBuffer.bufferSize;
                while (toRead > 0)
                {
                    DKASSERT_DEBUG(inputBuffer.bufferSize >= toRead);
                    size_t read = input->Read(inputBuffer.buffer, toRead);
                    if (read > 0)
                    {
                        ZSTD_inBuffer zInput = { inputBuffer.buffer, read, 0 };
                        while (zInput.pos < zInput.size)
                        {
                            ZSTD_outBuffer zOutput = { outputBuffer.buffer, outputBuffer.bufferSize, 0 };;
                            size_t toRead = ZSTD_compressStream(cstream, &zOutput, &zInput);
                            if (ZSTD_isError(toRead))
                            {
                                DKLogE("DKCompressor::Compress error: %s",
                                       ZSTD_getErrorName(toRead));
                                result = false;
                                break;
                            }
                            if (toRead > inputBuffer.bufferSize)
                                toRead = inputBuffer.bufferSize;
                            if (output->Write(outputBuffer.buffer, zOutput.pos) != zOutput.pos)
                            {
                                DKLogE("DKCompressor Error: Output stream error!");
                                result = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (read < 0) // error
                        {
                            DKLogE("DKCompressor::Compress error: Input stream error");
                            result = false;
                        }
                        break;
                    }
                }
                if (result)
                {
                    ZSTD_outBuffer zOutput = { outputBuffer.buffer, outputBuffer.bufferSize,0 };
                    size_t const remainingToFlush = ZSTD_endStream(cstream, &zOutput); // close frame.
                    if (remainingToFlush)
                    {
                        DKLogE("DKCompressor::Compress error: Unable to flush stream.");
                        result = false;
                    }
                    else
                    {
                        if (output->Write(outputBuffer.buffer, zOutput.pos) != zOutput.pos)
                        {
                            DKLogE("DKCompressor Error: Output stream error!");
                            result = false;
                        }
                    }
                }
            }
            ZSTD_freeCStream(cstream);
            return result;
        }
        else
        {
            DKLogE("DKCompressor::Compress error: ZSTD_createCStream failed");
        }
        return false;
    }

    static bool DecompressZstd(DKStream* input, DKStream* output)
    {
        CompressorBuffer inputBuffer(ZSTD_DStreamInSize());
        CompressorBuffer outputBuffer(ZSTD_DStreamOutSize());

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

        /*
        Zstd requests a large amount of memory allocation.
        So we do not need to use DKMalloc.
        */
#if 0   
        ZSTD_customMem customMem = {
            [](void* opaque, size_t size)->void* { return DKMalloc(size); }, //ZSTD_allocFunction
            [](void* opaque, void* address) { DKFree(address); }, //ZSTD_freeFunction
            nullptr //opaque
        };
        ZSTD_DStream* const dstream = ZSTD_createDStream_advanced(customMem);
#else
        ZSTD_DStream* const dstream = ZSTD_createDStream();
#endif
        if (dstream)
        {
            bool result = false;
            size_t const initResult = ZSTD_initDStream(dstream);
            if (ZSTD_isError(initResult))
            {
                DKLogE("DKCompressor::Compress error: ZSTD_initDStream failed: %s",
                       ZSTD_getErrorName(initResult));
            }
            else
            {
                result = true;
                size_t toRead = initResult;
                while (toRead > 0)
                {
                    DKASSERT_DEBUG(inputBuffer.bufferSize >= toRead);
                    size_t read = input->Read(inputBuffer.buffer, toRead);
                    if (read > 0)
                    {
                        ZSTD_inBuffer zInput = { inputBuffer.buffer, read, 0 };
                        while (zInput.pos < zInput.size)
                        {
                            ZSTD_outBuffer zOutput = { outputBuffer.buffer, outputBuffer.bufferSize,0 };
                            toRead = ZSTD_decompressStream(dstream, &zOutput, &zInput);
                            if (ZSTD_isError(toRead))
                            {
                                DKLogE("DKCompressor::Decompress error: %s",
                                       ZSTD_getErrorName(toRead));
                                result = false;
                                break;
                            }
                            if (output->Write(outputBuffer.buffer, zOutput.pos) != zOutput.pos)
                            {
                                DKLogE("DKCompressor Error: Output stream error!");
                                result = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (read < 0) // error
                        {
                            DKLogE("DKCompressor::Decompress error: Input stream error");
                            result = false;
                        }
                        break;
                    }
                }
            }

            ZSTD_freeDStream(dstream);
            return result;
        }
        else
        {
            DKLogE("DKCompressor::Decompress error: ZSTD_createDStream failed");
        }
        return false;
    }

    static bool CompressLZ4(DKStream* input, DKStream* output, int level)
    {
        LZ4F_preferences_t prefs = {};
        prefs.autoFlush = 1;
        prefs.compressionLevel = level;	// 0 for LZ4 fast, 9 for LZ4HC
        prefs.frameInfo.blockMode = LZ4F_blockLinked;	// for better compression ratio.
        prefs.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled; // to detect data corruption.
        prefs.frameInfo.blockSizeID = LZ4F_max4MB;

        size_t inputBufferSize = size_t(1) << (8 + (2 * prefs.frameInfo.blockSizeID));
        size_t outputBufferSize = LZ4F_compressFrameBound(inputBufferSize, &prefs);;

        CompressorBuffer inputBuffer(inputBufferSize);
        CompressorBuffer outputBuffer(outputBufferSize);

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

        LZ4F_compressionContext_t ctx;
        LZ4F_errorCode_t err = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);

        if (!LZ4F_isError(err))
        {
            bool result = false;

            size_t inputSize = input->Read(inputBuffer.buffer, inputBuffer.bufferSize);
            if (inputSize != DKStream::PositionError)
            {
                // generate header
                size_t headerSize = LZ4F_compressBegin(ctx, outputBuffer.buffer, outputBuffer.bufferSize, &prefs);
                if (!LZ4F_isError(headerSize))
                {
                    // write header
                    if (output->Write(outputBuffer.buffer, headerSize) == headerSize)
                    {
                        result = true;
                        // compress block
                        while (inputSize > 0)
                        {
                            size_t outputSize = LZ4F_compressUpdate(ctx, outputBuffer.buffer, outputBuffer.bufferSize, inputBuffer.buffer, inputSize, NULL);
                            if (LZ4F_isError(outputSize))
                            {
                                DKLog("DKCompressor Error: LZ4 Encoding error: %s", LZ4F_getErrorName(outputSize));
                                result = false;
                                break;
                            }
                            if (output->Write(outputBuffer.buffer, outputSize) != outputSize)
                            {
                                DKLog("DKCompressor Error: Output stream error!");
                                result = false;
                                break;
                            }
                            inputSize = input->Read(inputBuffer.buffer, inputBuffer.bufferSize);
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
                            headerSize = LZ4F_compressEnd(ctx, outputBuffer.buffer, outputBuffer.bufferSize, NULL);
                            if (!LZ4F_isError(headerSize))
                            {
                                // write footer
                                result = output->Write(outputBuffer.buffer, headerSize) == headerSize;
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
            err = LZ4F_freeCompressionContext(ctx);
            DKASSERT_DEBUG(!LZ4F_isError(err));
            return result;
        }
        else
        {
            DKLog("DKCompressor Error: LZ4 Encoder error: %s", LZ4F_getErrorName(err));
        }
        return false;
    }

    static bool DecompressLZ4(DKStream* input, DKStream* output)
    {
        CompressorBuffer inputBuffer(COMPRESSION_CHUNK_SIZE);
        CompressorBuffer outputBuffer(COMPRESSION_CHUNK_SIZE);

        if (inputBuffer.buffer == nullptr || outputBuffer.buffer == nullptr)
        {
            DKLogE("DKCompressor Error: Out of memory!");
            return false;
        }

        const uint32_t lz4_Header = DKSystemToLittleEndian(0x184D2204U);
        const uint32_t lz4_SkipHeader = DKSystemToLittleEndian(0x184D2A50U);

        LZ4F_decompressionContext_t ctx;
        LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
        if (!LZ4F_isError(err))
        {
            size_t inputSize = 0;
            size_t processed = 0;
            size_t inSize, outSize;
            uint8_t* const inData = reinterpret_cast<uint8_t*>(inputBuffer.buffer);
            bool decodeError = false;
            LZ4F_errorCode_t nextToLoad;

            while (!decodeError)
            {
                if (inputSize == 0)
                {
                    inputSize = input->Read(inputBuffer.buffer, inputBuffer.bufferSize);
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
                    do
                    {
                        while (processed < inputSize)
                        {
                            inSize = inputSize - processed;
                            outSize = outputBuffer.bufferSize;
                            nextToLoad = LZ4F_decompress(ctx, outputBuffer.buffer, &outSize, &inData[processed], &inSize, NULL);
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
                                if (output->Write(outputBuffer.buffer, outSize) != outSize)
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
                            inputSize = input->Read(inputBuffer.buffer, Min(nextToLoad, inputBuffer.bufferSize));
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

            return !decodeError;
        }
        return false;
    }

    static bool DetectMethod(void* p, size_t n, DKCompressor::Method& m)
    {
        if (p)
        {
            if (n >= 4)
            {
                const uint32_t zstd_MagicNumber = DKSystemToLittleEndian(0xFD2FB528U);

                if (reinterpret_cast<const uint32_t*>(p)[0] == zstd_MagicNumber)
                {
                    m = DKCompressor::Zstd;
                    return true;
                }
            }
            if (n >= 4)
            {
                const uint32_t lz4_Header = DKSystemToLittleEndian(0x184D2204U);
                const uint32_t lz4_SkipHeader = DKSystemToLittleEndian(0x184D2A50U);

                if (reinterpret_cast<const uint32_t*>(p)[0] == lz4_Header ||
                    (reinterpret_cast<const uint32_t*>(p)[0] & 0xfffffff0U) == lz4_SkipHeader)
                {
                    m = DKCompressor::LZ4;
                    return true;
                }
            }
            if (n >= 1)
            {
                if (reinterpret_cast<const char*>(p)[0] == 0x78)
                {
                    m = DKCompressor::Zlib;
                    return true;
                }
            }
        }
        return false;
    }

}
using namespace DKFoundation;
using namespace DKFoundation::Private;

DKCompressor::DKCompressor(Method m)
	: method(m)
{
}

DKCompressor::~DKCompressor()
{
}

bool DKCompressor::Compress(DKStream* input, DKStream* output) const
{
	if (input == NULL || input->IsReadable() == false)
		return false;
	if (output == NULL || output->IsWritable() == false)
		return false;

    switch (method)
    {
    case Zlib:
        return CompressDeflate(input, output, 5); // Z_DEFAULT_COMPRESSION is 6
        break;
    case Zstd:
        return CompressZstd(input, output, ZSTD_CLEVEL_DEFAULT);
        break;
    case ZstdMax:
        return CompressZstd(input, output, 19);// Clamp(19, int(ZSTD_CLEVEL_DEFAULT), ZSTD_maxCLevel()));
        break;
    case LZ4:
        return CompressLZ4(input, output, 0);
        break;
    case LZ4HC:
        return CompressLZ4(input, output, 9); // 0 for LZ4 fast, 9 for LZ4HC
        break;
    }
    DKLogE("DKCompressor::Compress error: Unknown format.");
    return false;
}

bool DKCompressor::Decompress(DKStream* input, DKStream* output)
{
	if (input == NULL || input->IsReadable() == false)
		return false;
	if (output == NULL || output->IsWritable() == false)
		return false;

    struct BufferedStream : public DKStream
    {
        enum {BufferLength = 1024};
        DKStream* source;
        uint8_t buffer[BufferLength];
        uint8_t* preloadedData;
        Position preloadedLength;

        BufferedStream(DKStream* s)
            : source(s)
        {
            preloadedLength = source->Read(buffer, BufferLength);
            if (preloadedLength > 0)
                preloadedData = buffer;
            else
                preloadedLength = 0; // error? 
        }
        Position SetCurrentPosition(Position p) override
        {
            auto pos = source->SetCurrentPosition(p);
            if (pos != PositionError)
                preloadedLength = 0;
            return pos;
        }
        Position CurrentPosition() const override
        {
            auto pos = source->CurrentPosition();
            return pos - preloadedLength;
        }
        Position RemainLength() const override
        {
            auto pos = source->RemainLength();
            return pos + preloadedLength;
        }
        Position TotalLength() const override
        {
            return source->TotalLength();
        }
        size_t Read(void* p, size_t s) override
        {
            size_t totalRead = 0;
            while (s > 0 && preloadedLength > 0)
            {
                size_t read = Min(s, preloadedLength);
                DKASSERT_DEBUG(read > 0);

                memcpy(p, preloadedData, read);
                preloadedData += read;
                preloadedLength -= read;

                DKASSERT_DEBUG(s >= read);

                s = s - read;
                p = &reinterpret_cast<uint8_t*>(p)[read];
                totalRead += read;
            }
            if (s > 0)
            {
                totalRead += source->Read(p, s);
            }
            DKASSERT_DEBUG(preloadedLength >= 0);
            return totalRead;
        }
        size_t Write(const void* p, size_t s) override
        {
            DKASSERT_DEBUG(0);
            return 0;
        }

        bool IsReadable() const override { return source->IsReadable(); }
        bool IsWritable() const override { return false;  }
        bool IsSeekable() const override { return source->IsSeekable(); }
    };
    BufferedStream bufferedInputStream(input);
    if (bufferedInputStream.preloadedLength <= 0)
    {
        DKLogE("DKCompressor Error: Cannot read input source");
        return false;
    }

    Method method;
    if (DetectMethod(bufferedInputStream.preloadedData, bufferedInputStream.preloadedLength, method))
    {
        switch (method)
        {
        case Zlib:
            return DecompressDeflate(&bufferedInputStream, output);
            break;
        case Zstd:
        case ZstdMax:
            return DecompressZstd(&bufferedInputStream, output);
            break;
        case LZ4:
        case LZ4HC:
            return DecompressLZ4(&bufferedInputStream, output);
            break;
        }
    }
    DKLogE("DKCompressor Error: Unknown format!");
    return false;
}
