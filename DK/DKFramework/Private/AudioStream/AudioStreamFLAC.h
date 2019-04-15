//
//  File: AudioStreamFLAC.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2010-2019 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKAudioStream.h"

///////////////////////////////////////////////////////////////////////////////
// AudioStreamFLAC
// Audio stream class inherited from AudioStream.
// This class supports audio stream for FLAC format.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework::Private
{
    struct FLAC_Context;
    class AudioStreamFLAC : public DKAudioStream
    {
    public:
        AudioStreamFLAC();
        virtual ~AudioStreamFLAC();

        virtual bool Open(const DKString& file);
        virtual bool Open(DKStream* stream);

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
    protected:
        AudioStreamFLAC(bool isOGG);
        bool InitMetadata();
        FLAC_Context* context;
    };

    class AudioStreamOggFLAC : public AudioStreamFLAC
    {
    public:
        AudioStreamOggFLAC();
        ~AudioStreamOggFLAC();
        bool Open(const DKString& file);
        bool Open(DKStream* stream);
    };
}
