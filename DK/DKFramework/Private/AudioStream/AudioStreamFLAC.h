//
//  File: AudioStreamFLAC.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2010-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamFLAC
// Audio stream class inherited from AudioStream.
// This class supports audio stream for FLAC format.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	namespace Private
	{
		struct FLAC_Context;
		class AudioStreamFLAC : public DKAudioStream
		{
		public:
			AudioStreamFLAC(void);
			virtual ~AudioStreamFLAC(void);

			virtual bool Open(const DKString& file);
			virtual bool Open(DKStream* stream);

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
		protected:
			AudioStreamFLAC(bool isOGG);
			bool InitMetadata(void);
			FLAC_Context* context;
		};

		class AudioStreamOggFLAC : public AudioStreamFLAC
		{
		public:
			AudioStreamOggFLAC(void);
			~AudioStreamOggFLAC(void);
			bool Open(const DKString& file);
			bool Open(DKStream* stream);
		};
	}
}
