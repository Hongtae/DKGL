//
//  File: DKAudioStreamVorbis.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// DKAudioStreamVorbis
// Audio stream class inherited from DKAudioStream.
// This class supports audio stream for OGG-Vorbis format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	namespace Private
	{
		struct VorbisFileContext;
		class DKAudioStreamVorbis : public DKAudioStream
		{
		public:
			DKAudioStreamVorbis(void);
			~DKAudioStreamVorbis(void);

			bool Open(const DKFoundation::DKString& file);
			bool Open(DKFoundation::DKStream* stream);

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
