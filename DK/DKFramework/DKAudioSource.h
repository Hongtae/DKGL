//
//  File: DKAudioSource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector3.h"

namespace DKFramework
{
	/// @brief 
	/// audio source class. interface for DKAudioDevice source control.
	/// @details
	/// Audio-Source can play audio with input buffer
	class DKGL_API DKAudioSource
	{
	public:
		enum AudioState
		{
			StateUnknown = 0,
			StateStopped,
			StatePlaying,
			StatePaused,
		};

		DKAudioSource();
		virtual ~DKAudioSource();

		bool Play();
		bool Stop();
		bool Pause();

		/// enqueue audio stream buffer
		bool EnqueueBuffer(int frequency, int bits, int channels, const void* data, size_t bytes, double timeStamp);
		void UnqueueBuffers() const;
		size_t QueuedBuffers() const;

		double TimePosition() const;
		void SetTimePosition(double t);
		double TimeOffset() const;
		void SetTimeOffset(double t);

		AudioState State() const;

		// pitch control
		void SetPitch(float f);
		float Pitch() const;

		// audio gain
		void SetGain(float f);
		float Gain() const;

		// min gain
		void SetMinGain(float f);
		float MinGain() const;

		// max gain
		void SetMaxGain(float f);
		float MaxGain() const;

		// max distance
		void SetMaxDistance(float f);
		float MaxDistance() const;

		// rolloff factor
		void SetRolloffFactor(float f);
		float RolloffFactor() const;

		// cone outer gain
		void SetConeOuterGain(float f);
		float ConeOuterGain() const;

		// cone inner angle
		void SetConeInnerAngle(float f);
		float ConeInnerAngle() const;

		// cone outer angle
		void SetConeOuterAngle(float f);
		float ConeOuterAngle() const;

		// reference distance
		void SetReferenceDistance(float f);
		float ReferenceDistance() const;

		// source position
		void SetPosition(const DKVector3& v);
		DKVector3 Position() const;

		// source velocity
		void SetVelocity(const DKVector3& v);
		DKVector3 Velocity() const;

		// source direction
		void SetDirection(const DKVector3& v);
		DKVector3 Direction() const;


		static int Format(int bits, int channels);
		static int ErrorCode();
		static DKString ErrorString(int errCode);
		
	private:
		unsigned int sourceId;

		struct BufferInfo
		{
			double timeStamp;
			size_t bytes;
			size_t bytesSecond;
			unsigned int bufferId;
		};
		DKSpinLock bufferLock;
		mutable DKArray<BufferInfo> buffers;
	};
}
