//
//  File: DKAudioSource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKAudioSource
// audio source class. interface for OpenAL source control.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKAudioSource
	{
	public:
		enum AudioState
		{
			StateUnknown = 0,
			StateStopped,
			StatePlaying,
			StatePaused,
		};

		DKAudioSource(void);
		virtual ~DKAudioSource(void);

		bool Play(void);
		bool Stop(void);
		bool Pause(void);

		bool EnqueueBuffer(int frequency, int bits, int channels, const void* data, size_t bytes, double timeStamp);
		void UnqueueBuffers(void) const;
		size_t QueuedBuffers(void) const;

		double TimePosition(void) const;
		void SetTimePosition(double t);
		double TimeOffset(void) const;
		void SetTimeOffset(double t);

		AudioState State(void) const;

		// pitch control
		void SetPitch(float f);
		float Pitch(void) const;

		// audio gain
		void SetGain(float f);
		float Gain(void) const;

		// min gain
		void SetMinGain(float f);
		float MinGain(void) const;

		// max gain
		void SetMaxGain(float f);
		float MaxGain(void) const;

		// max distance
		void SetMaxDistance(float f);
		float MaxDistance(void) const;

		// rolloff factor
		void SetRolloffFactor(float f);
		float RolloffFactor(void) const;

		// cone outer gain
		void SetConeOuterGain(float f);
		float ConeOuterGain(void) const;

		// cone inner angle
		void SetConeInnerAngle(float f);
		float ConeInnerAngle(void) const;

		// cone outer angle
		void SetConeOuterAngle(float f);
		float ConeOuterAngle(void) const;

		// reference distance
		void SetReferenceDistance(float f);
		float ReferenceDistance(void) const;

		// source position
		void SetPosition(const DKVector3& v);
		DKVector3 Position(void) const;

		// source velocity
		void SetVelocity(const DKVector3& v);
		DKVector3 Velocity(void) const;

		// source direction
		void SetDirection(const DKVector3& v);
		DKVector3 Direction(void) const;


		static int Format(int bits, int channels);
		static int ErrorCode(void);
		static DKFoundation::DKString ErrorString(int errCode);
		
	private:
		unsigned int sourceId;

		struct BufferInfo
		{
			double timeStamp;
			size_t bytes;
			size_t bytesSecond;
			unsigned int bufferId;
		};
		DKFoundation::DKSpinLock bufferLock;
		mutable DKFoundation::DKArray<BufferInfo> buffers;
	};
}
