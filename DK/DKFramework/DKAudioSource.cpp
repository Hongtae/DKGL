//
//  File: DKAudioSource.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include <float.h>
#include "../lib/Inc_OpenAL.h"
#include "DKAudioSource.h"
#include "DKOpenALContext.h"

using namespace DKGL;
using namespace DKGL;

DKAudioSource::DKAudioSource(void)
	: sourceId(0)
{
	alGenSources(1, &sourceId);
	alSourcei(sourceId, AL_LOOPING, 0);
	alSourcei(sourceId, AL_BUFFER, 0);
	alSourceStop(sourceId);

	DKASSERT_DEBUG(this->sourceId != 0);
}

DKAudioSource::~DKAudioSource(void)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	DKASSERT_DEBUG(alIsSource(sourceId));

	Stop();

	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	DKASSERT_DEBUG(buffers.Count() == 0);
	alDeleteSources(1, &sourceId);
	sourceId = 0;

#ifdef DKGL_DEBUG_ENABLED
	// check error.
	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
	}
#endif
}

bool DKAudioSource::Play(void)
{
	DKASSERT_DEBUG(this->sourceId != 0);

	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	alSourcePlay(sourceId);
	return true;
}

bool DKAudioSource::Stop(void)
{
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	DKASSERT_DEBUG(this->sourceId != 0);

	alSourceStop(sourceId);
	ALint buffersQueued = 0;
	ALint buffersProcessed = 0;
	alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &buffersQueued);		 // entire buffer
	alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &buffersProcessed); // finished buffer

	for (int i = 0; i < buffersProcessed; ++i)
	{
		ALuint bufferId = 0;
		alSourceUnqueueBuffers(sourceId, 1, &bufferId);
	}

#ifdef DKGL_DEBUG_ENABLED
	if (buffersProcessed != buffers.Count())
	{
		DKLog("DKAudioSource Warning: Buffer mismatch! (%d allocated, %d released)\n", (int)buffers.Count(), (int)buffersProcessed); 
	}
#endif

	alSourcei(sourceId, AL_LOOPING, 0);
	alSourcei(sourceId, AL_BUFFER, 0);
	alSourceRewind(sourceId);

	for (size_t i = 0; i < buffers.Count(); ++i)
	{
		ALuint bufferId = buffers.Value(i).bufferId;
		alDeleteBuffers(1, &bufferId);
	}
	buffers.Clear();

#ifdef DKGL_DEBUG_ENABLED
	// check error.
	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
		DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
#endif

	return true;
}

bool DKAudioSource::Pause(void)
{
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	DKASSERT_DEBUG(this->sourceId != 0);

	alSourcePause(sourceId);
	return false;
}

bool DKAudioSource::EnqueueBuffer(int frequency, int bits, int channels, const void* data, size_t bytes, double timeStamp)
{
	DKASSERT_DEBUG(this->sourceId != 0);

	if (data && bytes > 0 && frequency > 0)
	{
		int format = Format(bits, channels);
		if (format != 0)
		{
			DKCriticalSection<DKSpinLock> guard(this->bufferLock);

			DKArray<ALuint> finishedBuffers;
			ALint numBuffersProcessed = 0;
			alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &numBuffersProcessed);
			finishedBuffers.Reserve(numBuffersProcessed);
			while (numBuffersProcessed > 0)
			{
				ALuint bufferId = 0;
				alSourceUnqueueBuffers(sourceId, 1, &bufferId); // collect buffer to recycle
				if (bufferId)
				{
					finishedBuffers.Add(bufferId);
				}
				numBuffersProcessed--;
			}

			ALuint bufferId = 0;

			if (finishedBuffers.Count() > 0)
			{
				for (size_t i = 0; i < finishedBuffers.Count(); ++i)
				{
					bufferId = finishedBuffers.Value(i);
					for (size_t k = 0; k < this->buffers.Count(); ++k)
					{
						if (this->buffers.Value(k).bufferId == bufferId)
						{
							this->buffers.Remove(k);
							break;
						}
						//DKLog("DKAudioSource buffer unqueued. total:%d\n", this->buffers.Count());
					}
				}
				bufferId = finishedBuffers.Value(0);
				ALsizei numBuffers = (ALsizei)finishedBuffers.Count();
				if (numBuffers > 1)
				{
					ALuint* buff = finishedBuffers;
					alDeleteBuffers(numBuffers - 1, &buff[1]);
				}
			}
			if (bufferId == 0)
			{
				alGenBuffers(1, &bufferId);
			}
			// enqueue buffer.
			alBufferData(bufferId, format, data, (ALsizei)bytes, frequency);
			alSourceQueueBuffers(sourceId, 1, &bufferId);

			if (true)
			{
				size_t bytesSecond = frequency * channels * (bits / 8);
				BufferInfo info = {timeStamp, bytes, bytesSecond, bufferId};
				this->buffers.Add(info);

				//DKLog("DKAudioSource buffer queued. total:%d\n", this->buffers.Count());
			}
#ifdef DKGL_DEBUG_ENABLED
			// check error.
			ALenum err = alGetError();
			if (err != AL_NO_ERROR)
			{
				DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
			}
#endif
			return true;
		}
		else
		{
			DKLog("DKAudioSource Error: Wrong format!\n");
		}
	}

	UnqueueBuffers();
	return false;
}

void DKAudioSource::UnqueueBuffers(void) const
{
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	DKASSERT_DEBUG(this->sourceId != 0);

	ALint bufferProcessed = 0;
	alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &bufferProcessed);
	for (int i = 0; i < bufferProcessed; ++i)
	{
		ALuint bufferId = 0;
		alSourceUnqueueBuffers(sourceId, 1, &bufferId);

		if (bufferId)
		{
			for (size_t i = 0; i < buffers.Count(); ++i)
			{
				if (buffers.Value(i).bufferId == bufferId)
				{
					buffers.Remove(i);
					break;
				}
			}
			alDeleteBuffers(1, &bufferId);
		}
		else
		{
			DKLog("DKAudioSource Failed to unqueue buffer! (source:0x%x)\n", sourceId);
		}

#ifdef DKGL_DEBUG_ENABLED
		// check error.
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
		}
#endif
	}

	if (bufferProcessed > 0)
	{
	//	DKLog("DKAudioSource buffer unqueued. total:%d\n", this->buffers.Count());
	}
}

size_t DKAudioSource::QueuedBuffers(void) const
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);

	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
#ifdef DKGL_DEBUG_ENABLED
	// get number of total buffers.
	ALint queuedBuffers = 0;
	alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queuedBuffers);
	if (queuedBuffers != this->buffers.Count())
	{
		DKLog("DKAudioSource buffer count mismatch! (%d != %d)\n", buffers.Count(), queuedBuffers);
	}
#endif

	return buffers.Count();
}

double DKAudioSource::TimePosition(void) const
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);

	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	if (this->buffers.Count())
	{
		const BufferInfo& buffInfo = buffers.Value(0);
		DKASSERT_DEBUG(buffInfo.bufferId != 0);
		DKASSERT_DEBUG(buffInfo.bytes != 0);
		DKASSERT_DEBUG(buffInfo.bytesSecond != 0);

		ALint bytesOffset = 0;
		alGetSourcei(sourceId, AL_BYTE_OFFSET, &bytesOffset);
		// If last buffer is too small, playing over next buffer before unqueue.
		// This can be time accuracy problem.
		bytesOffset = Clamp(bytesOffset, 0, (ALint)buffInfo.bytes);

		double position = buffInfo.timeStamp + (static_cast<double>(bytesOffset) / static_cast<double>(buffInfo.bytesSecond));
		return position;
	}
	return 0;
}

void DKAudioSource::SetTimePosition(double t)
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);
	
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	if (this->buffers.Count())
	{
		const BufferInfo& buffInfo = buffers.Value(0);
		DKASSERT_DEBUG(buffInfo.bufferId != 0);
		DKASSERT_DEBUG(buffInfo.bytes != 0);
		DKASSERT_DEBUG(buffInfo.bytesSecond != 0);
		
		if (t > buffInfo.timeStamp)
		{
			t -= buffInfo.timeStamp;
			ALint bytesOffset = Clamp<ALint>((buffInfo.bytesSecond * t), 0, (ALint)buffInfo.bytes);
			
			alSourcef(sourceId, AL_BYTE_OFFSET, bytesOffset);

#ifdef DKGL_DEBUG_ENABLED
			// check error.
			ALenum err = alGetError();
			if (err != AL_NO_ERROR)
			{
				DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
			}
#endif
		}
	}
}

double DKAudioSource::TimeOffset(void) const
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);
	
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	if (this->buffers.Count())
	{
		const BufferInfo& buffInfo = buffers.Value(0);
		DKASSERT_DEBUG(buffInfo.bufferId != 0);
		DKASSERT_DEBUG(buffInfo.bytes != 0);
		DKASSERT_DEBUG(buffInfo.bytesSecond != 0);
		
		ALint bytesOffset = 0;
		alGetSourcei(sourceId, AL_BYTE_OFFSET, &bytesOffset);
		// If last buffer is too small, playing over next buffer before unqueue.
		// This can be time accuracy problem.
		bytesOffset = Clamp(bytesOffset, 0, (ALint)buffInfo.bytes);
		
		return static_cast<double>(bytesOffset) / static_cast<double>(buffInfo.bytesSecond);
	}
	return 0;	
}

void DKAudioSource::SetTimeOffset(double t)
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);
	
	DKCriticalSection<DKSpinLock> guard(this->bufferLock);
	if (this->buffers.Count())
	{
		const BufferInfo& buffInfo = buffers.Value(0);
		DKASSERT_DEBUG(buffInfo.bufferId != 0);
		DKASSERT_DEBUG(buffInfo.bytes != 0);
		DKASSERT_DEBUG(buffInfo.bytesSecond != 0);
		
		ALint bytesOffset = Clamp<ALint>(t * buffInfo.bytesSecond, 0, (ALint)buffInfo.bytes);
		alSourcef(sourceId, AL_BYTE_OFFSET, bytesOffset);

#ifdef DKGL_DEBUG_ENABLED
		// check error.
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			DKLog("DKAudioSource Error:0x%x (%s)\n", err, alGetString(err));
		}
#endif
	}	
}

DKAudioSource::AudioState DKAudioSource::State(void) const
{
	this->UnqueueBuffers();
	DKASSERT_DEBUG(this->sourceId != 0);
	
	// check source state.
	ALint st = 0;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &st);
	if (st == AL_PLAYING)
		return StatePlaying;
	if (st == AL_PAUSED)
		return StatePaused;
	return StateStopped;
}

void DKAudioSource::SetPitch(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Max(f, 0.0);
	alSourcef(this->sourceId, AL_PITCH, f);
}

float DKAudioSource::Pitch(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 1.0;
	alGetSourcef(this->sourceId, AL_PITCH, &f);
	return f;
}

void DKAudioSource::SetGain(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Max(f, 0.0);
	alSourcef(sourceId, AL_GAIN, f);
}

float DKAudioSource::Gain(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 1.0;
	alGetSourcef(this->sourceId, AL_GAIN, &f);
	return f;
}

void DKAudioSource::SetMinGain(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Clamp(f, 0.0, 1.0);
	alSourcef(sourceId, AL_MIN_GAIN, f);
}

float DKAudioSource::MinGain(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 0.0;
	alGetSourcef(this->sourceId, AL_MIN_GAIN, &f);
	return f;
}

void DKAudioSource::SetMaxGain(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Clamp(f, 0.0, 1.0);
	alSourcef(sourceId, AL_MAX_GAIN, f);
}

float DKAudioSource::MaxGain(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 1.0;
	alGetSourcef(this->sourceId, AL_MAX_GAIN, &f);
	return f;
}

void DKAudioSource::SetMaxDistance(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Max(f, 0.0);
	alSourcef(sourceId, AL_MAX_DISTANCE, f);
}

float DKAudioSource::MaxDistance(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = FLT_MAX;
	alGetSourcef(this->sourceId, AL_MAX_DISTANCE, &f);
	return f;
}

void DKAudioSource::SetRolloffFactor(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Max(f, 0.0);
	alSourcef(this->sourceId, AL_ROLLOFF_FACTOR, f);
}

float DKAudioSource::RolloffFactor(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 1.0;
	alGetSourcef(this->sourceId, AL_ROLLOFF_FACTOR, &f);
	return f;
}

void DKAudioSource::SetConeOuterGain(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Clamp(f, 0.0, 1.0);
	alSourcef(this->sourceId, AL_CONE_OUTER_GAIN, f);
}

float DKAudioSource::ConeOuterGain(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 0.0;
	alGetSourcef(this->sourceId, AL_CONE_OUTER_GAIN, &f);
	return f;
}

void DKAudioSource::SetConeInnerAngle(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Clamp(f, 0.0, 360.0);
	alSourcef(this->sourceId, AL_CONE_INNER_ANGLE, f);
}

float DKAudioSource::ConeInnerAngle(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 360.0;
	alGetSourcef(this->sourceId, AL_CONE_INNER_ANGLE, &f);
	return f;
}

void DKAudioSource::SetConeOuterAngle(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Clamp(f, 0.0, 360.0);
	alSourcef(this->sourceId, AL_CONE_OUTER_ANGLE, f);
}

float DKAudioSource::ConeOuterAngle(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 360.0;
	alGetSourcef(this->sourceId, AL_CONE_OUTER_ANGLE, &f);
	return f;
}

void DKAudioSource::SetReferenceDistance(float f)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	f = Max(f, 0.0);
	alSourcef(this->sourceId, AL_REFERENCE_DISTANCE, f);
}

float DKAudioSource::ReferenceDistance(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	float f = 1.0;
	alGetSourcef(this->sourceId, AL_REFERENCE_DISTANCE, &f);
	return f;
}

void DKAudioSource::SetPosition(const DKVector3& v)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	alSource3f(this->sourceId, AL_POSITION, v.x, v.y, v.z);
}

DKVector3 DKAudioSource::Position(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	DKVector3 v(0,0,0);
	alGetSource3f(this->sourceId, AL_POSITION, &v.x, &v.y, &v.z);
	return v;
}

void DKAudioSource::SetVelocity(const DKVector3& v)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	alSource3f(sourceId, AL_VELOCITY, v.x, v.y, v.z);
}

DKVector3 DKAudioSource::Velocity(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	DKVector3 v(0, 0, 0);
	alGetSource3f(this->sourceId, AL_VELOCITY, &v.x, &v.y, &v.z);
	return v;
}

void DKAudioSource::SetDirection(const DKVector3& v)
{
	DKASSERT_DEBUG(this->sourceId != 0);
	alSource3f(sourceId, AL_DIRECTION, v.x, v.y, v.z);
}

DKVector3 DKAudioSource::Direction(void) const
{
	DKASSERT_DEBUG(this->sourceId != 0);
	DKVector3 v(0, 0, 0);
	alGetSource3f(this->sourceId, AL_DIRECTION, &v.x, &v.y, &v.z);
	return v;
}

int DKAudioSource::Format(int bits, int channels)
{
	auto formatKey = [](unsigned short bits, unsigned short channels) -> unsigned int
	{
		union
		{
			struct
			{
				unsigned short bits;
				unsigned short channels;
			} bc;
			unsigned int value;
		} val = {bits, channels};
		return val.value;
	};

	typedef DKMap<unsigned int, int> FormatTable;
	FormatTable* formatTable = NULL;
	static DKSpinLock lock;
	
	if (formatTable == NULL)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		if (formatTable == NULL)
		{
			static FormatTable fmts;
			
			DKObject<DKOpenALContext> alContext = DKOpenALContext::SharedInstance();
			alContext->Bind();
			
			fmts.Update(formatKey( 4, 1), alGetEnumValue("AL_FORMAT_MONO_IMA4"));
			fmts.Update(formatKey( 4, 2), alGetEnumValue("AL_FORMAT_STEREO_IMA4"));
			
			fmts.Update(formatKey( 8, 1), AL_FORMAT_MONO8);
			fmts.Update(formatKey( 8, 2), AL_FORMAT_STEREO8);
			fmts.Update(formatKey( 8, 4), alGetEnumValue("AL_FORMAT_QUAD8"));
			fmts.Update(formatKey( 8, 6), alGetEnumValue("AL_FORMAT_51CHN8"));
			fmts.Update(formatKey( 8, 8), alGetEnumValue("AL_FORMAT_71CHN8"));
			
			fmts.Update(formatKey(16, 1), AL_FORMAT_MONO16);
			fmts.Update(formatKey(16, 2), AL_FORMAT_STEREO16);
			fmts.Update(formatKey(16, 4), alGetEnumValue("AL_FORMAT_QUAD16"));
			fmts.Update(formatKey(16, 6), alGetEnumValue("AL_FORMAT_51CHN16"));
			fmts.Update(formatKey(16, 8), alGetEnumValue("AL_FORMAT_71CHN16"));
			
			fmts.Update(formatKey(32, 1), alGetEnumValue("AL_FORMAT_MONO_FLOAT32"));
			fmts.Update(formatKey(32, 2), alGetEnumValue("AL_FORMAT_STEREO_FLOAT32"));
			fmts.Update(formatKey(32, 4), alGetEnumValue("AL_FORMAT_QUAD32"));
			fmts.Update(formatKey(32, 6), alGetEnumValue("AL_FORMAT_51CHN32"));
			fmts.Update(formatKey(32, 8), alGetEnumValue("AL_FORMAT_71CHN32"));
			
			alContext->Unbind();
			
			formatTable = &fmts;
		}
	}
	
	DKASSERT_DEBUG(formatTable != NULL);
	if (bits < 0xFFFF && channels < 0xFFFF)
	{
		FormatTable::Pair* p = formatTable->Find(formatKey(bits, channels));
		if (p)
			return p->value;
	}
	DKLog("Unsupported audio format! (%d bps, %%d channels)\n", bits, channels);
	return 0;
}

int DKAudioSource::ErrorCode(void)
{
	return alGetError();
}

DKString DKAudioSource::ErrorString(int errCode)
{
	return DKString(alGetString(errCode));
}
