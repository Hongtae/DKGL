//
//  File: AudioStreamWave.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamWave
// Audio stream class inherited from AudioStream.
// This class supports audio stream for Wave format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	namespace Private
	{
		struct WaveFileContext;
		class AudioStreamWave : public DKAudioStream
		{
		public:
			AudioStreamWave(void);
			~AudioStreamWave(void);

			bool Open(const DKString& file);
			bool Open(DKStream* stream);

			size_t Read(void* pBuffer, size_t nSize);

			Position SeekRaw(Position nPos);
			Position SeekPcm(Position nPos);
			double SeekTime(double s);

			Position RawPos(void) const;
			Position PcmPos(void) const;
			double TimePos(void) const;

			Position RawTotal(void) const;
			Position PcmTotal(void) const;
			double TimeTotal(void) const;
		private:
			WaveFileContext* context;
		};
	}
}
