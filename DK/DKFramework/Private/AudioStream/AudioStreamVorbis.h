//
//  File: AudioStreamVorbis.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamVorbis
// Audio stream class inherited from AudioStream.
// This class supports audio stream for OGG-Vorbis format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	namespace Private
	{
		struct VorbisFileContext;
		class AudioStreamVorbis : public DKAudioStream
		{
		public:
			AudioStreamVorbis();
			~AudioStreamVorbis();

			bool Open(const DKString& file);
			bool Open(DKStream* stream);

			size_t Read(void* buffer, size_t size);

			Position SeekRaw(Position pos);
			Position SeekPcm(Position pos);
			double SeekTime(double s);

			Position RawPos() const;
			Position PcmPos() const;
			double TimePos() const;

			Position RawTotal() const;
			Position PcmTotal() const;
			double TimeTotal() const;
		private:
			VorbisFileContext* context;
		};
	}
}
