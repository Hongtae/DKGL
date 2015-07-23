//
//  File: DKAudioStreamFLAC.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2010-2014 Hongtae Kim. All rights reserved.
//

#define DKGL_EXTDEPS_LIBFLAC
#include <memory.h>
#include <math.h>
#include "../../../lib/ExtDeps.h"
#include "DKAudioStreamFLAC.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		struct FLAC_Context
		{
			FLAC_Context(void) : decoder(NULL), stream(NULL), totalSamples(0), sampleNumber(0), sampleRate(0), channels(0), bps(0) {}
			FLAC__StreamDecoder* decoder;
			DKObject<DKStream> stream;

			unsigned long long totalSamples;
			unsigned long long sampleNumber;
			unsigned int sampleRate;
			unsigned int channels;
			unsigned int bps;

			DKArray<FLAC__int32> buffer;
		};

		FLAC__StreamDecoderReadStatus FLAC_Read(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->stream)
			{
				if (*bytes > 0)
				{
					*bytes = ctxt->stream->Read(buffer, *bytes);
					if (*bytes == (size_t)-1)
						return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
					else if (*bytes == 0)
						return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
					else
						return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
				}
			}
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		}

		FLAC__StreamDecoderSeekStatus FLAC_Seek(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->stream)
			{
				if (ctxt->stream->IsSeekable())
				{
					DKStream::Position pos = ctxt->stream->SetPos(absolute_byte_offset);
					if (pos == absolute_byte_offset)
						return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
					return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
				}
				return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
			}
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		}

		FLAC__StreamDecoderTellStatus FLAC_Tell(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->stream)
			{
				if (ctxt->stream->IsSeekable())
				{
					*absolute_byte_offset = (FLAC__uint64)ctxt->stream->GetPos();
					return FLAC__STREAM_DECODER_TELL_STATUS_OK;
				}
				return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
			}
			return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
		}

		FLAC__StreamDecoderLengthStatus FLAC_Length(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->stream)
			{
				if (ctxt->stream->IsSeekable())
				{
					*stream_length = (FLAC__uint64)ctxt->stream->TotalLength();
					return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
				}
				return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
			}
			return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
		}

		FLAC__bool FLAC_IsEOF(const FLAC__StreamDecoder *decoder, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->stream)
			{
				return (ctxt->stream->RemainLength() > 0) ? false : true;
			}
			return false;
		}

		FLAC__StreamDecoderWriteStatus FLAC_Write(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			if (ctxt->channels == frame->header.channels && ctxt->bps == frame->header.bits_per_sample && ctxt->sampleRate == frame->header.sample_rate)
			{
				// add to audio buffer.
				size_t blockSize = frame->header.blocksize;
				size_t buffSize = ctxt->buffer.Count();

				ctxt->sampleNumber = frame->header.number.sample_number;
				ctxt->buffer.Reserve(buffSize + (blockSize * frame->header.channels));
				for (unsigned int i = 0; i < frame->header.blocksize; ++i)
				{
					for (unsigned int ch = 0; ch < frame->header.channels; ++ch)
					{
						ctxt->buffer.Add(buffer[ch][i]);
					}
				}
				return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
			}
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}

		void FLAC_Metadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			DKASSERT_DEBUG(ctxt != NULL);

			DKLog("FLAC_Metadata (contxt:%p):%s \n", ctxt, FLAC__MetadataTypeString[metadata->type]);

			if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
			{
				ctxt->totalSamples = metadata->data.stream_info.total_samples;
				ctxt->sampleRate = metadata->data.stream_info.sample_rate;
				ctxt->channels = metadata->data.stream_info.channels;
				ctxt->bps = metadata->data.stream_info.bits_per_sample;

				DKLog("FLAC_Metadata total samples: %llu\n", ctxt->totalSamples);
				DKLog("FLAC_Metadata sample rate: %u Hz\n", ctxt->sampleRate);
				DKLog("FLAC_Metadata channels: %u\n", ctxt->channels);
				DKLog("FLAC_Metadata bits per sample: %u\n", ctxt->bps);
			}
		}

		void FLAC_Error(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
		{
			FLAC_Context* ctxt = reinterpret_cast<FLAC_Context*>(client_data);
			DKLog("FLAC_Error (context:%p): %s\n", ctxt, FLAC__StreamDecoderErrorStatusString[status]);
		}
	}
}


using namespace DKFramework;
using namespace DKFramework::Private;


DKAudioStreamFLAC::DKAudioStreamFLAC(void)
	: DKAudioStream(DKAudioStream::FileTypeFLAC)
	, context(new FLAC_Context())
{
}

DKAudioStreamFLAC::DKAudioStreamFLAC(bool isOGG)
	: DKAudioStream( isOGG ? DKAudioStream::FileTypeOggFLAC : DKAudioStream::FileTypeFLAC)
	, context(new FLAC_Context())
{
}

DKAudioStreamFLAC::~DKAudioStreamFLAC(void)
{
	if (context->decoder)
	{
		FLAC__stream_decoder_finish(context->decoder);
		FLAC__stream_decoder_delete(context->decoder);
	}
	delete context;
}

bool DKAudioStreamFLAC::Open(const DKString& file)
{
	DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenReadOnly, DKFile::ModeShareRead);
	if (f)
	{
		return Open(f.SafeCast<DKStream>());
	}
	return false;
}

bool DKAudioStreamFLAC::Open(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		context->stream = stream;
		context->decoder = FLAC__stream_decoder_new();

		FLAC__StreamDecoderInitStatus st = FLAC__stream_decoder_init_stream(context->decoder,
			&Private::FLAC_Read,
			&Private::FLAC_Seek,
			&Private::FLAC_Tell,
			&Private::FLAC_Length,
			&Private::FLAC_IsEOF,
			&Private::FLAC_Write,
			&Private::FLAC_Metadata,
			&Private::FLAC_Error,
			context);

		if (st == FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			if (InitMetadata())
			{
				return true;
			}
			else
			{
			}
			FLAC__stream_decoder_finish(context->decoder);
		}
		else
		{
			DKLog("FLAC__stream_decoder_init_stream failed:%s\n", FLAC__StreamDecoderInitStatusString[st]);
		}
		FLAC__stream_decoder_delete(context->decoder);
		context->stream = NULL;
		context->decoder = NULL;
	}
	return false;
}

bool DKAudioStreamFLAC::InitMetadata(void)
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context->decoder);

	if (FLAC__stream_decoder_process_until_end_of_metadata(context->decoder))
	{
		if ((context->bps == 8 || context->bps == 16 || context->bps == 24) &&
			(context->totalSamples > 0 && context->sampleRate > 0 && context->channels > 0))
		{
			SetChannels(context->channels);
			SetFrequency(context->sampleRate);
			SetSeekable(context->stream->IsSeekable());

			switch (context->bps)
			{
			case 8:
			case 16:
				SetBits(context->bps);
			case 24:
				SetBits(16);		// convert to 16 bits internally.
				break;
			default:
				DKLog("FLAC Unsupported bps:%u.\n", context->bps);
				return false;
			}
			return true;
		}
		else
		{
			DKLog("FLAC Unsupported stream! (bps:%u, freq:%u, channels:%u)\n", context->bps, context->sampleRate, context->channels);
		}
	}
	else
	{
		FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
		DKLog("FLAC__stream_decoder_process_until_end_of_metadata failed. (state:%s)\n", FLAC__StreamDecoderStateString[st]);
	}
	return false;
}

size_t DKAudioStreamFLAC::Read(void* buffer, size_t size)
{
	if (context->decoder)
	{
		// reading until buffer become full
		while ( context->buffer.Count() < size )
		{
			if (FLAC__stream_decoder_process_single(context->decoder))
			{
				FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
				if (st == FLAC__STREAM_DECODER_END_OF_STREAM || st == FLAC__STREAM_DECODER_ABORTED)
				{
					DKLog("FLAC State:%s\n", FLAC__StreamDecoderStateString[st]);
					break;
				}
			}
			else
			{
				FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
				DKLog("FLAC__stream_decoder_process_single failed. (state:%s)\n", FLAC__StreamDecoderStateString[st]);
				break;
			}
		}

		size_t numSamples = context->buffer.Count();	
		if (numSamples > 0)
		{
			size_t copiedSamples = 0;
			size_t bytesCopied = 0;

			FLAC__int32* p = (FLAC__int32*)context->buffer;

			if (context->bps == 8)
			{
				for (copiedSamples = 0; (bytesCopied + 1) < size && copiedSamples < numSamples ; ++copiedSamples)
				{
					reinterpret_cast<FLAC__int8*>(buffer)[copiedSamples] = static_cast<FLAC__int8>(p[copiedSamples]);
					bytesCopied += 1;
				}
			}
			else if (context->bps == 16)
			{
				for (copiedSamples = 0; (bytesCopied + 2) <= size && copiedSamples < numSamples ; ++copiedSamples)
				{
					reinterpret_cast<FLAC__int16*>(buffer)[copiedSamples] = static_cast<FLAC__int16>(p[copiedSamples]);
					bytesCopied += 2;
				}			
			}
			else if (context->bps == 24)
			{
				static_assert(sizeof(FLAC__int16) == sizeof(short), "size mismatch?");

				for (copiedSamples = 0; (bytesCopied + 2) <= size && copiedSamples < numSamples; ++copiedSamples)
				{
					// float has 23bits fraction (on IEEE754)
					// which can have int24(23+1) without loss.
					float sig = static_cast<float>(p[copiedSamples]);		
					sig = (sig / float(1<<23)) * float(1<<15);				// int24 -> float -> int16.
					FLAC__int16 sample = (FLAC__int16)Clamp<int>( (sig+0.5), -32768, 32767);
					reinterpret_cast<FLAC__int16*>(buffer)[copiedSamples] = sample;
					bytesCopied += 2;
				}				
			}
			else
			{
				DKERROR_THROW_DEBUG("Unsupported bps!");
				DKLog("FLAC: Unsupported bps!\n");
				context->buffer.Clear();
				return -1;
			}
			context->buffer.Remove(0, copiedSamples);
			return bytesCopied;
		}
	}
	return -1;
}

DKAudioStream::Position DKAudioStreamFLAC::SeekRaw(Position pos)
{
	if (context->decoder)
	{
		pos = (pos / context->channels) / (context->bps / 8);		// raw to pcm(sample)
		pos = Clamp(pos, 0, context->totalSamples);
		if (FLAC__stream_decoder_seek_absolute(context->decoder, pos))
		{
			FLAC__stream_decoder_process_single(context->decoder);
			return pos * context->channels * (context->bps / 8);
		}
		else
		{
			FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
			DKLog("FLAC__stream_decoder_process_until_end_of_metadata failed:%s\n", FLAC__StreamDecoderStateString[st]);
			if (st == FLAC__STREAM_DECODER_SEEK_ERROR)
				FLAC__stream_decoder_flush(context->decoder);
		}
	}
	return 0;
}

DKAudioStream::Position DKAudioStreamFLAC::SeekPcm(Position pos)
{
	if (context->decoder)
	{
		pos = Clamp(pos, 0, context->totalSamples);
		if (FLAC__stream_decoder_seek_absolute(context->decoder, pos))
		{
			FLAC__stream_decoder_process_single(context->decoder);
			return pos;
		}
		else
		{
			FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
			DKLog("FLAC__stream_decoder_process_until_end_of_metadata failed:%s\n", FLAC__StreamDecoderStateString[st]);
			if (st == FLAC__STREAM_DECODER_SEEK_ERROR)
				FLAC__stream_decoder_flush(context->decoder);
		}
	}
	return 0;
}

double DKAudioStreamFLAC::SeekTime(double s)
{
	if (context->decoder)
	{
		FLAC__uint64 pos = s * context->sampleRate;
		pos = Clamp(pos, 0, context->totalSamples);
		if (FLAC__stream_decoder_seek_absolute(context->decoder, pos))
		{
			FLAC__stream_decoder_process_single(context->decoder);
			return static_cast<double>(pos) / context->sampleRate;
		}
		else
		{
			FLAC__StreamDecoderState st = FLAC__stream_decoder_get_state(context->decoder);
			DKLog("FLAC__stream_decoder_process_until_end_of_metadata failed:%s\n", FLAC__StreamDecoderStateString[st]);
			if (st == FLAC__STREAM_DECODER_SEEK_ERROR)
				FLAC__stream_decoder_flush(context->decoder);
		}
	}
	return 0;
}

DKAudioStream::Position DKAudioStreamFLAC::RawPos(void) const
{
	if (context->decoder)
	{
		Position pos = context->sampleNumber;
		Position samplesRemain = context->buffer.Count() / context->channels;
		if (pos >= samplesRemain)
			pos -= samplesRemain;
		return pos * context->channels * (context->bps / 8);
	}
	return 0;
}

DKAudioStream::Position DKAudioStreamFLAC::PcmPos(void) const
{
	if (context->decoder)
	{
		Position pos = context->sampleNumber;
		Position samplesRemain = context->buffer.Count() / context->channels;
		if (pos >= samplesRemain)
			pos -= samplesRemain;
		return pos;
	}
	return 0;
}

double DKAudioStreamFLAC::TimePos(void) const
{
	if (context->decoder)
	{
		Position pos = context->sampleNumber;
		Position samplesRemain = context->buffer.Count() / context->channels;
		if (pos >= samplesRemain)
			pos -= samplesRemain;
		return static_cast<double>(pos) / static_cast<double>(context->sampleRate);
	}
	return 0;
}

DKAudioStream::Position DKAudioStreamFLAC::RawTotal(void) const
{
	if (context->decoder)
	{
		return context->totalSamples * context->channels * (context->bps / 8);
	}
	return 0;
}

DKAudioStream::Position DKAudioStreamFLAC::PcmTotal(void) const
{
	if (context->decoder)
	{
		return context->totalSamples;
	}
	return 0;
}

double DKAudioStreamFLAC::TimeTotal(void) const
{
	if (context->decoder)
	{
		return static_cast<double>(context->totalSamples) / static_cast<double>(context->sampleRate);
	}
	return 0;
}

DKAudioStreamOggFLAC::DKAudioStreamOggFLAC(void)
	: DKAudioStreamFLAC(true)
{
}

DKAudioStreamOggFLAC::~DKAudioStreamOggFLAC(void)
{
}

bool DKAudioStreamOggFLAC::Open(const DKString& file)
{
	DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenReadOnly, DKFile::ModeShareRead);
	if (f)
	{
		return Open(f.SafeCast<DKStream>());
	}
	return false;
}

bool DKAudioStreamOggFLAC::Open(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		context->stream = stream;
		context->decoder = FLAC__stream_decoder_new();

		FLAC__StreamDecoderInitStatus st = FLAC__stream_decoder_init_ogg_stream(context->decoder,
			&Private::FLAC_Read,
			&Private::FLAC_Seek,
			&Private::FLAC_Tell,
			&Private::FLAC_Length,
			&Private::FLAC_IsEOF,
			&Private::FLAC_Write,
			&Private::FLAC_Metadata,
			&Private::FLAC_Error,
			context);

		if (st == FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			if (InitMetadata())
			{
				return true;
			}
			else
			{
			}
			FLAC__stream_decoder_finish(context->decoder);
		}
		else
		{
			DKLog("FLAC__stream_decoder_init_stream failed:%s\n", FLAC__StreamDecoderInitStatusString[st]);
		}
		FLAC__stream_decoder_delete(context->decoder);
		context->stream = NULL;
		context->decoder = NULL;
	}
	return false;
}
