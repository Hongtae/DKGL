//
//  File: DKAudioPlayer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"
#include "DKAudioStream.h"
#include "DKAudioSource.h"

namespace DKFramework
{
	/// @brief
	/// Audio player class.
	/// @details
	/// Using DKAudioSource internally. Provides control interface.
	/// To set-up 3d audio environments, you have to get DKAudioSource instance by
	/// calling DKAudioPlayer::AudioSource().
	class DKGL_API DKAudioPlayer
	{
	public:
		/// PCM audio buffer filter.
		/// a function or function object which prototype must be 'void (data, size, time)'
		typedef DKFunctionSignature<void (void*, size_t, double)>	StreamFilter;
		
		typedef DKAudioSource::AudioState AudioState;

		DKAudioPlayer();
		virtual ~DKAudioPlayer();

		/// create instance from data stream.
		static DKObject<DKAudioPlayer> Create(DKStream* stream);
		/// create instance from file.
		static DKObject<DKAudioPlayer> Create(const DKString& file);
		/// create instance from audio stream.
		/// @note stream should not be shared.
		static DKObject<DKAudioPlayer> Create(DKAudioStream* stream);

		int Channels() const;
		int Bits() const;
		int SampleRate() const;
		double Duration() const;

		double TimePosition() const;
		AudioState State() const;

		DKAudioStream* Stream()							{return stream;}
		const DKAudioSource* Source() const				{return source;}

		void PlayLoop(double pos, int loops);
		void Play();
		void Stop();
		void Pause();

		DKAudioSource* AudioSource();
		const DKAudioSource* AudioSource() const;

		/// set filter.
		void SetStreamFilter(StreamFilter* f);
		
		/// buffer control. (0 for minimum size, smaller buffer could be laggy)
		void SetBufferingTime(double t);
		double BufferingTime() const;

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
