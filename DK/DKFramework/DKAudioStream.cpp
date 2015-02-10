//
//  File: DKAudioStream.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKAudioStream.h"
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "Private/DKAudioStreamVorbis.h"
#include "Private/DKAudioStreamFLAC.h"
#include "Private/DKAudioStreamWave.h"

#define AUDIO_FORMAT_HEADER_LENGTH		35

using namespace DKFoundation;
using namespace DKFramework;

DKAudioStream::DKAudioStream(FileType t)
	: type(t)
{
}

DKAudioStream::~DKAudioStream(void)
{
}

DKAudioStream::FileType DKAudioStream::DetermineAudioType(char* data, size_t len)
{
	if (len >= 4 && memcmp(data, "OggS", 4) == 0)
	{
		// vorbis or flac.
		if (len >= 32 && memcmp(&data[29], "fLaC", 4) == 0)
		{
			return FileTypeOggFLAC;
		}
		if (len >= 35 && memcmp(&data[29], "vorbis", 6) == 0)
		{
			return FileTypeOggVorbis;
		}
		return FileTypeUnknown;
	}
	else if (len >= 4 && memcmp(data, "fLaC", 4) == 0)
	{
		return FileTypeFLAC;
	}
	else if (len >= 4 && memcmp(data, "RIFF", 4) == 0)
	{
		return FileTypeWave;
	}
	return FileTypeUnknown;
}

DKObject<DKAudioStream> DKAudioStream::Create(DKStream* stream)
{
	if (stream && stream->IsReadable() && stream->IsSeekable())
	{
		stream->SetPos(0);

		// reading file header.
		char header[AUDIO_FORMAT_HEADER_LENGTH];
		memset(header, 0, AUDIO_FORMAT_HEADER_LENGTH);
		stream->Read(header, AUDIO_FORMAT_HEADER_LENGTH);
		stream->SetPos(0);

		FileType type = DetermineAudioType(header, AUDIO_FORMAT_HEADER_LENGTH);

		if (type == FileTypeOggVorbis)
		{
			DKObject<Private::DKAudioStreamVorbis> audioStream = DKObject<Private::DKAudioStreamVorbis>::New();
			if (audioStream->Open(stream))
				return audioStream.SafeCast<DKAudioStream>();

			return NULL;
		}
		else if (type == FileTypeOggFLAC)
		{
			DKObject<Private::DKAudioStreamOggFLAC> audioStream = DKObject<Private::DKAudioStreamOggFLAC>::New();
			if (audioStream->Open(stream))
				return audioStream.SafeCast<DKAudioStream>();

			return NULL;
		}
		else if (type == FileTypeFLAC)
		{
			DKObject<Private::DKAudioStreamFLAC> audioStream = DKObject<Private::DKAudioStreamFLAC>::New();
			if (audioStream->Open(stream))
				return audioStream.SafeCast<DKAudioStream>();

			return NULL;
		}
		else if (type == FileTypeWave)
		{
			DKObject<Private::DKAudioStreamWave> audioStream = DKObject<Private::DKAudioStreamWave>::New();
			if (audioStream->Open(stream))
				return audioStream.SafeCast<DKAudioStream>();

			return NULL;
		}
	}
	return NULL;
}

DKObject<DKAudioStream> DKAudioStream::Create(const DKFoundation::DKString& file)
{
	DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll);
	if (f == NULL)
		return NULL;

	// reading file header.
	char header[AUDIO_FORMAT_HEADER_LENGTH];
	memset(header, 0, AUDIO_FORMAT_HEADER_LENGTH);

	f->Read(header, AUDIO_FORMAT_HEADER_LENGTH);
	f = NULL;

	FileType type = DetermineAudioType(header, AUDIO_FORMAT_HEADER_LENGTH);

	if (type == FileTypeOggVorbis)
	{
		DKObject<Private::DKAudioStreamVorbis> audioStream = DKObject<Private::DKAudioStreamVorbis>::New();
		if (audioStream->Open(file))
			return audioStream.SafeCast<DKAudioStream>();
	}
	else if (type == FileTypeOggFLAC)
	{
		DKObject<Private::DKAudioStreamOggFLAC> audioStream = DKObject<Private::DKAudioStreamOggFLAC>::New();
		if (audioStream->Open(file))
			return audioStream.SafeCast<DKAudioStream>();

		return NULL;
	}
	else if (type == FileTypeFLAC)
	{
		DKObject<Private::DKAudioStreamFLAC> audioStream = DKObject<Private::DKAudioStreamFLAC>::New();
		if (audioStream->Open(file))
			return audioStream.SafeCast<DKAudioStream>();

		return NULL;
	}
	else if (type == FileTypeWave)
	{
		DKObject<Private::DKAudioStreamWave> audioStream = DKObject<Private::DKAudioStreamWave>::New();
		if (audioStream->Open(file))
			return audioStream.SafeCast<DKAudioStream>();

		return NULL;
	}
	return NULL;
}
