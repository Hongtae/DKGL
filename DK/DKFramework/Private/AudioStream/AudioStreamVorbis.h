//
//  File: AudioStreamVorbis.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamVorbis
// Audio stream class inherited from AudioStream.
// This class supports audio stream for OGG-Vorbis format.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	namespace Private
	{
		struct VorbisFileContext;
		class AudioStreamVorbis : public DKAudioStream
		{
		public:
			AudioStreamVorbis(void);
			~AudioStreamVorbis(void);

			bool Open(const DKString& file);
			bool Open(DKStream* stream);

			size_t Read(void* buffer, size_t size);

			Position SeekRaw(Position pos);
			Position SeekPcm(Position pos);
			double SeekTime(double s);

			Position RawPos(void) const;
			Position PcmPos(void) const;
			double TimePos(void) const;

			Position RawTotal(void) const;
			Position PcmTotal(void) const;
			double TimeTotal(void) const;
		private:
			VorbisFileContext* context;
		};
	}
}
