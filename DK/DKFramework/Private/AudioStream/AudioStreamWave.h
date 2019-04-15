//
//  File: AudioStreamWave.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamWave
// Audio stream class inherited from AudioStream.
// This class supports audio stream for Wave format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework::Private
{
    struct WaveFileContext;
    class AudioStreamWave : public DKAudioStream
    {
    public:
        AudioStreamWave();
        ~AudioStreamWave();

        bool Open(const DKString& file);
        bool Open(DKStream* stream);

        size_t Read(void* pBuffer, size_t nSize);

        Position SeekRaw(Position nPos);
        Position SeekPcm(Position nPos);
        double SeekTime(double s);

        Position RawPos() const;
        Position PcmPos() const;
        double TimePos() const;

        Position RawTotal() const;
        Position PcmTotal() const;
        double TimeTotal() const;
    private:
        WaveFileContext* context;
    };
}
