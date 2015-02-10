//
//  File: DKAudioStreamFLAC.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2010-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// DKAudioStreamFLAC
// Audio stream class inherited from DKAudioStream.
// This class supports audio stream for FLAC format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	namespace Private
	{
		struct FLAC_Context;
		class DKAudioStreamFLAC : public DKAudioStream
		{
		public:
			DKAudioStreamFLAC(void);
			virtual ~DKAudioStreamFLAC(void);

			virtual bool Open(const DKFoundation::DKString& file);
			virtual bool Open(DKFoundation::DKStream* stream);

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
			DKAudioStreamFLAC(bool isOGG);
			bool InitMetadata(void);
			FLAC_Context* context;
		};

		class DKAudioStreamOggFLAC : public DKAudioStreamFLAC
		{
		public:
			DKAudioStreamOggFLAC(void);
			~DKAudioStreamOggFLAC(void);
			bool Open(const DKFoundation::DKString& file);
			bool Open(DKFoundation::DKStream* stream);
		};
	}
}
