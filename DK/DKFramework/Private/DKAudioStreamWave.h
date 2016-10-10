//
//  File: DKAudioStreamWave.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// DKAudioStreamWave
// Audio stream class inherited from DKAudioStream.
// This class supports audio stream for Wave format.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	namespace Private
	{
		struct WaveFileContext;
		class DKAudioStreamWave : public DKAudioStream
		{
		public:
			DKAudioStreamWave(void);
			~DKAudioStreamWave(void);

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
