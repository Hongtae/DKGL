//
//  File: DKAudioPlayer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKAudioStream.h"
#include "DKAudioSource.h"

////////////////////////////////////////////////////////////////////////////////
// DKAudioPlayer
// Audio player class.
// Using DKAudioSource internally. Provides control interface.
// To set-up 3d audio environments, you have to get DKAudioSource instance by
// calling DKAudioPlayer::AudioSource().
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKAudioPlayer
	{
	public:
		// PCM audio buffer filter.
		// a function or function object which prototype must be 'void (data, size, time)'
		typedef DKFunctionSignature<void (void*, size_t, double)>	StreamFilter;
		
		typedef DKAudioSource::AudioState AudioState;

		DKAudioPlayer(void);
		virtual ~DKAudioPlayer(void);

		// create instance from data stream.
		static DKObject<DKAudioPlayer> Create(DKStream* stream);
		// create instance from file.
		static DKObject<DKAudioPlayer> Create(const DKString& file);
		// create instance from audio stream.
		// Note: stream should not be shared.
		static DKObject<DKAudioPlayer> Create(DKAudioStream* stream);

		int Channels(void) const;
		int Bits(void) const;
		int Frequency(void) const;
		double Duration(void) const;

		double TimePosition(void) const;
		AudioState State(void) const;

		DKAudioStream* Stream(void)							{return stream;}
		const DKAudioSource* Source(void) const				{return source;}

		void PlayLoop(double pos, int loops);
		void Play(void);
		void Stop(void);
		void Pause(void);

		DKAudioSource* AudioSource(void);
		const DKAudioSource* AudioSource(void) const;

		// set filter.
		void SetStreamFilter(StreamFilter* f);
		
		// buffer control. (0 for minimum size, smaller buffer could be laggy)
		void SetBufferingTime(double t);
		double BufferingTime(void) const;

	private:
		class AudioQueue;

		DKObject<StreamFilter> filter;

		double			bufferingTime;
		double			timePosition;
		double			duration;

		void UpdatePlaybackState(int ps, double tp);
		void UpdateBufferState(int bs, double tp);

		void ProcessStream(void *data, size_t size, double time); // invoked by AudioController.

		AudioState	playerState;
		int			queuePlaybackState;
		int			queueBufferState;
		DKObject<DKAudioStream>	stream; // audio data
		DKObject<DKAudioSource>	source;
		DKObject<AudioQueue>		queue;
		DKSpinLock				lock;
	};
}
