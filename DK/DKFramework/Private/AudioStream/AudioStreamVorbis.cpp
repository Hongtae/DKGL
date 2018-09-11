//
//  File: AudioStreamVorbis.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include <ogg/ogg.h>
#include "../../../Libs/libvorbis/include/vorbis/codec.h"
#include "../../../Libs/libvorbis/include/vorbis/vorbisfile.h"
#include "AudioStreamVorbis.h"

#define SWAP_CHANNEL16(x, y)		{short t = x; x = y ; y = t;}

using namespace DKFramework;

namespace DKFramework
{
	namespace Private
	{
		struct VorbisStream
		{
			DKObject<DKStream>	stream;
			size_t currentPos;
		};
		size_t VorbisStreamRead(void *ptr, size_t size, size_t nmemb, void *datasource)
		{
			VorbisStream *pSource = reinterpret_cast<VorbisStream*>(datasource);

			size_t validSize = size*nmemb;

			return pSource->stream->Read(ptr, validSize);
		}
		int VorbisStreamSeek(void *datasource, ogg_int64_t offset, int whence)
		{
			VorbisStream *pSource = reinterpret_cast<VorbisStream*>(datasource);
			switch (whence)
			{
				case SEEK_SET:
					return pSource->stream->SetCurrentPosition(offset);
					break;
				case SEEK_CUR:
					return pSource->stream->SetCurrentPosition(pSource->stream->CurrentPosition() + offset);
					break;
				case SEEK_END:
					return pSource->stream->SetCurrentPosition(pSource->stream->TotalLength() + offset);
					break;
			}
			return -1;
		}
		int VorbisStreamClose(void *datasource)
		{
			VorbisStream *pSource = reinterpret_cast<VorbisStream*>(datasource);
			pSource->stream = NULL;
			return 0;
		}
		long VorbisStreamTell(void *datasource)
		{
			return reinterpret_cast<VorbisStream*>(datasource)->stream->CurrentPosition();
		}
		struct VorbisFileContext
		{
			OggVorbis_File		vorbis;
			VorbisStream*		stream;
		};
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;


AudioStreamVorbis::AudioStreamVorbis()
	: DKAudioStream(DKAudioStream::FileTypeOggVorbis)
	, context(new VorbisFileContext())
{
	memset(context, 0, sizeof(VorbisFileContext));
}

AudioStreamVorbis::~AudioStreamVorbis()
{
	if (context->vorbis.datasource)
		ov_clear(&context->vorbis);
	delete context->stream;
	delete context;
}

bool AudioStreamVorbis::Open(const DKString& file)
{
	if (context->vorbis.datasource)
		ov_clear(&context->vorbis);
	delete context->stream;
	context->stream = NULL;

	DKStringU8 tmp(file);
	if (tmp.Bytes() == 0)
		return false;

	if (ov_fopen((char*)(const char*)tmp, &context->vorbis) == 0)
	{
		vorbis_info *pInfo = ov_info(&context->vorbis, -1);
		if (pInfo)
		{
			SetChannels(pInfo->channels);
			SetFrequency(pInfo->rate);
			SetSeekable((bool)ov_seekable(&context->vorbis));
			SetBits(16);
			return true;
		}
	}
	return false;
}

bool AudioStreamVorbis::Open(DKStream* stream)
{
	if (stream == NULL || !stream->IsReadable())
		return false;

	if (context->vorbis.datasource)
		ov_clear(&context->vorbis);
	delete context->stream;
	context->stream = NULL;

	VorbisStream *pSource = new VorbisStream();
	pSource->stream = stream;
	pSource->currentPos = 0;

	ov_callbacks	ogg_callbacks;
	ogg_callbacks.read_func = VorbisStreamRead;
	ogg_callbacks.seek_func = VorbisStreamSeek;
	ogg_callbacks.tell_func = VorbisStreamTell;
	ogg_callbacks.close_func = VorbisStreamClose;

	if (ov_open_callbacks(pSource, &context->vorbis, 0, 0, ogg_callbacks) == 0)
	{
		vorbis_info *pInfo = ov_info(&context->vorbis, -1);
		if (pInfo)
		{
			SetChannels(pInfo->channels);
			SetFrequency(pInfo->rate);
			SetSeekable((bool)ov_seekable(&context->vorbis));
			SetBits(16);
			context->stream = pSource;
			return true;
		}
	}
	delete pSource;
	return false;
}

size_t AudioStreamVorbis::Read(void* buffer, size_t size)
{
	if (context->vorbis.datasource == NULL)
		return -1;
	if (size == 0)
		return 0;

	int current_section;
	int nDecoded = 0;
	while (nDecoded < size)
	{
		int nDec = ov_read(&context->vorbis, (char*)buffer + nDecoded, size - nDecoded, 0,2,1, &current_section);
		if (nDec <= 0)
		{
			// error or eof.
			break;
		}
		nDecoded += nDec;
	}

	if (Channels() == 6)
	{
		short *p = (short*)buffer;
		for ( int i = 0; i < nDecoded / 2; i+=6)
		{
			SWAP_CHANNEL16(p[i+1], p[i+2]);
			SWAP_CHANNEL16(p[i+3], p[i+5]);
			SWAP_CHANNEL16(p[i+4], p[i+5]);
		}
	}
	return nDecoded;
}

DKAudioStream::Position AudioStreamVorbis::SeekRaw(Position pos)
{
	if (context->vorbis.datasource == NULL)
		return -1;

	ov_raw_seek(&context->vorbis, pos);
	return ov_raw_tell(&context->vorbis);
}

DKAudioStream::Position AudioStreamVorbis::SeekPcm(Position pos)
{
	if (context->vorbis.datasource == NULL)
		return -1;

	ov_pcm_seek(&context->vorbis, pos);
	return ov_pcm_tell(&context->vorbis);
}

double AudioStreamVorbis::SeekTime(double s)
{
	if (context->vorbis.datasource == NULL)
		return -1;

	ov_time_seek(&context->vorbis, s);
	return ov_time_tell(&context->vorbis);
}

DKAudioStream::Position AudioStreamVorbis::RawPos() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_raw_tell(&context->vorbis);
}

DKAudioStream::Position AudioStreamVorbis::PcmPos() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_pcm_tell(&context->vorbis);
}

double AudioStreamVorbis::TimePos() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_time_tell(&context->vorbis);
}

DKAudioStream::Position AudioStreamVorbis::RawTotal() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_raw_total(&context->vorbis, -1);
}

DKAudioStream::Position AudioStreamVorbis::PcmTotal() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_pcm_total(&context->vorbis, -1);
}

double AudioStreamVorbis::TimeTotal() const
{
	if (context->vorbis.datasource == NULL)
		return -1;

	return ov_time_total(&context->vorbis, -1);
}
