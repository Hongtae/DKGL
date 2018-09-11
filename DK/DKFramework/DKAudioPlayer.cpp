//
//  File: DKAudioPlayer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/OpenAL.h"
#include "../DKFoundation.h"
#include "DKAudioPlayer.h"
#include "DKAudioStream.h"
#include "DKAudioSource.h"
#include "DKAudioDevice.h"

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			enum QueueStatePlayback
			{
				QueueStatePlaybackStopped = 0,
				QueueStatePlaybackPlaying,
			};
			enum QueueStateBuffer
			{
				QueueStateBufferStopped = 0,
				QueueStateBufferFeeding,
			};
			typedef DKAudioSource::AudioState AudioState;
			typedef DKFunctionSignature<void(void*, size_t, double)> StreamCallback;
			typedef DKFunctionSignature<void(int, double)> StateCallback;

			struct SourceStream
			{
				DKObject<DKAudioSource> source;
				DKObject<DKAudioStream> stream;
				DKObject<StreamCallback> streamCallback;
				DKObject<StateCallback> playbackStateCallback;
				DKObject<StateCallback> bufferStateCallback;
				int loops;
				bool playing;    // set by AudioQueue.
				bool buffering;  // set by AudioQueue.
				double bufferPos;
				double playbackPos;
				size_t bufferSize;
				DKObject<DKOperation> request;
			};
			typedef DKMap<void*, SourceStream> SourceStreamMap;
			typedef DKCriticalSection<DKSpinLock> CriticalSection;
			SourceStreamMap		sourceStreamMap;
			DKSpinLock			queueLock;
			DKCondition			playbackCond;
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

class DKAudioPlayer::AudioQueue : public DKSharedInstance<AudioQueue>
{
public:
	enum { MaxBufferCount = 3 };
	AudioQueue() : terminate(false), activeSources(0)
	{
		playbackThread = DKThread::Create(DKFunction(this, &AudioQueue::Playback)->Invocation());
	}
	~AudioQueue()
	{
		DKASSERT_DEBUG(playbackThread && playbackThread->IsAlive());
		terminate = true;
		playbackCond.Signal();
		playbackThread->WaitTerminate();
		playbackThread = NULL;

		if (true)
		{
			CriticalSection guard(queueLock);
			DKASSERT_DEBUG(this->activeSources == 0);
			DKASSERT_DEBUG(sourceStreamMap.Count() == 0);
		}
	}
private:
	void FeedBuffer(SourceStreamMap::Pair& p)
	{
		SourceStream& ss = p.value;
		if (ss.request)
			ss.request->Perform();
		ss.request = NULL;

		if (ss.bufferSize > 0)
		{
			if (ss.source->QueuedBuffers() < MaxBufferCount)
			{
				buffer.Resize(ss.bufferSize);
				unsigned char* buff = buffer;

				ss.bufferPos = ss.stream->TimePos();
				size_t bytesRead = ss.stream->Read(buff, ss.bufferSize);
				if (bytesRead > 0 && bytesRead != (size_t)-1)
				{
					if (!ss.buffering)
					{
						ss.buffering = true;
						if (ss.bufferStateCallback)
							this->operations.Add((DKOperation*)ss.bufferStateCallback->Invocation(QueueStateBufferFeeding, ss.bufferPos));
					}

					if (ss.streamCallback)
						ss.streamCallback->Invoke(buff, bytesRead, ss.bufferPos);
					if (ss.source->EnqueueBuffer(ss.stream->Frequency(), ss.stream->Bits(), ss.stream->Channels(), buff, bytesRead, ss.bufferPos))
					{
						if (ss.source->State() != DKAudioSource::StatePlaying)
							ss.source->Play();

						ss.playing = true;
					}
					else		// EnqueueBuffer failed.
					{
						DKLog("AudioQueue: buffer enqueue failed!\n");

						ss.buffering = false;
						ss.playing = false;
						if (ss.bufferStateCallback)
							this->operations.Add((DKOperation*)ss.bufferStateCallback->Invocation(QueueStateBufferStopped, ss.bufferPos));
						if (ss.playbackStateCallback)
							this->operations.Add((DKOperation*)ss.playbackStateCallback->Invocation(QueueStatePlaybackStopped, ss.playbackPos));
					}
				}
				else	// buffering finished.
				{
					ss.source->UnqueueBuffers();

					if (ss.loops > 1)
					{
						ss.loops--;
						ss.stream->SeekRaw(0);		// rewind
						if (!ss.playing)
						{
							ss.playing = true;
							if (ss.playbackStateCallback)
								this->operations.Add((DKOperation*)ss.playbackStateCallback->Invocation(QueueStatePlaybackPlaying, ss.playbackPos));
						}
					}
					else
					{
						if (ss.buffering)
						{
							ss.buffering = false;
							if (ss.bufferStateCallback)
								this->operations.Add((DKOperation*)ss.bufferStateCallback->Invocation(QueueStateBufferStopped, ss.bufferPos));
						}

						if (ss.source->State() != DKAudioSource::StatePlaying)
						{
							ss.playing = false;
							if (ss.playbackStateCallback)
								this->operations.Add((DKOperation*)ss.playbackStateCallback->Invocation(QueueStatePlaybackStopped, ss.playbackPos));
						}
					}
				}
			}
		}
		else
		{
			ss.source->UnqueueBuffers();

			if (ss.source->State() != DKAudioSource::StatePlaying)
			{
				ss.playing = false;
				if (ss.playbackStateCallback)
					this->operations.Add((DKOperation*)ss.playbackStateCallback->Invocation(QueueStatePlaybackStopped, ss.playbackPos));
			}

			if (ss.buffering)
			{
				ss.buffering = false;
				if (ss.bufferStateCallback)
					this->operations.Add((DKOperation*)ss.bufferStateCallback->Invocation(QueueStateBufferStopped, ss.bufferPos));
			}
		}
		if (ss.playing)
		{
			ss.playbackPos = ss.source->TimePosition();
			if (ss.playbackStateCallback)
				this->operations.Add((DKOperation*)ss.playbackStateCallback->Invocation(QueueStatePlaybackPlaying, ss.playbackPos));
			if (ss.source->State() == DKAudioSource::StatePlaying)
				this->activeSources++;
		}
	}
	void Cleanup(SourceStreamMap::Pair& p)
	{
		SourceStream& ss = p.value;
		if (ss.request)
			ss.request->Perform();
		ss.request = NULL;
		ss.source->Stop();
		ss.source->UnqueueBuffers();
	}
	void Playback()
	{
		DKLog("AudioQueue thread initialized.\n");

		DKObject<DKAudioDevice> alContext = DKAudioDevice::SharedInstance();
		alContext->Bind();

		playbackCond.Lock();
		while (!terminate)
		{
			if (this->activeSources > 0)
				playbackCond.WaitTimeout(0.01);
			else
				playbackCond.Wait();

			this->activeSources = 0;
			operations.Clear();

			CriticalSection section(queueLock);
			sourceStreamMap.EnumerateForward([this](SourceStreamMap::Pair& pair) {this->FeedBuffer(pair); });

			for (size_t i = 0; i < operations.Count(); ++i)
				operations.Value(i)->Perform();
			operations.Clear();
		}
		CriticalSection section(queueLock);
		sourceStreamMap.EnumerateForward([this](SourceStreamMap::Pair& pair) {this->Cleanup(pair); });
		sourceStreamMap.Clear();

		playbackCond.Unlock();
		alContext->Unbind();

		DKASSERT_DEBUG(this->activeSources == 0);

		DKLog("AudioQueue thread terminated.\n");
	}
private:
	DKArray<unsigned char> buffer;
	DKArray<DKObject<DKOperation>> operations;
	DKObject<DKThread>	playbackThread;
	size_t activeSources;
	bool terminate;
};

DKAudioPlayer::DKAudioPlayer()
: queue(NULL)
, source(NULL)
, stream(0)
, playerState(AudioState::StateUnknown)
, queuePlaybackState(0)
, queueBufferState(0)
, filter(NULL)
, bufferingTime(1.0)
, duration(0.0)
, timePosition(0.0)
{
	queue = AudioQueue::SharedInstance();
}

DKAudioPlayer::~DKAudioPlayer()
{
	if (true)
	{
		Private::CriticalSection guard(Private::queueLock);
		Private::SourceStreamMap::Pair* p = Private::sourceStreamMap.Find(this);
		if (p)
		{
			sourceStreamMap.Remove(this);
			playbackCond.Signal();
			//	DKLog("PLAYBACK-QUEUE COUNT:%d\n", sourceStreamMap.Count());
		}
	}
	this->queue = NULL;
	this->stream = NULL;
	this->source = NULL;
}

DKObject<DKAudioPlayer> DKAudioPlayer::Create(DKStream* stream)
{
	return Create(DKAudioStream::Create(stream));
}

DKObject<DKAudioPlayer> DKAudioPlayer::Create(const DKString& file)
{
	return Create(DKAudioStream::Create(file));
}

DKObject<DKAudioPlayer> DKAudioPlayer::Create(DKAudioStream* stream)
{
	if (stream)
	{
		DKObject<DKAudioPlayer>	player = DKObject<DKAudioPlayer>::New();
		player->stream = stream;
		player->source = DKObject<DKAudioSource>::New();
		player->playerState = AudioState::StateStopped;
		player->timePosition = 0;
		player->bufferingTime = 1.0;
		player->duration = stream->TimeTotal();

		Private::CriticalSection guard(Private::queueLock);
		Private::SourceStream ss = {
			player->source,
			player->stream,
			DKFunction((DKAudioPlayer*)player, &DKAudioPlayer::ProcessStream),
			DKFunction((DKAudioPlayer*)player, &DKAudioPlayer::UpdatePlaybackState),
			DKFunction((DKAudioPlayer*)player, &DKAudioPlayer::UpdateBufferState),
			0,
			false,
			false,
			0.0,
			0.0,
			0			// bufferSize (0 for don't play now)
		};

		sourceStreamMap.Update(player, ss);

		return player;
	}
	return NULL;
}

void DKAudioPlayer::PlayLoop(double pos, int loops)
{
	if (this->source && this->stream && loops > 0)
	{
		Private::CriticalSection guard1(Private::queueLock);
		Private::CriticalSection guard2(this->lock);
		Private::SourceStreamMap::Pair* p = sourceStreamMap.Find(this);
		if (p)
		{
			Private::SourceStream& ss = p->value;

			if (!ss.playing)
			{
				size_t oneSecLength = stream->Frequency() * stream->Channels() * (stream->Bits() / 8);
				size_t baseAlignment = stream->Channels() * (stream->Bits() / 8);

				size_t desiredLength = static_cast<size_t>(static_cast<double>(oneSecLength)* bufferingTime);
				size_t pp = desiredLength % baseAlignment;
				if (pp)
					desiredLength += baseAlignment - pp;

				ss.playing = true;
				ss.bufferSize = desiredLength;
				ss.stream->SeekTime(pos);
				ss.loops = loops;
				ss.playbackPos = pos;
				ss.bufferPos = 0.0;
			}
			else
			{
				ss.request = DKFunction(this->source, &DKAudioSource::Play)->Invocation();
			}
			playbackCond.Signal();
			playerState = AudioState::StatePlaying;
		}
		else
		{
			DKLog("DKAudioPlayer(0x%x) is invalid.\n", this);
		}
	}
}

void DKAudioPlayer::Play()
{
	if (this->source && this->stream)
	{
		Private::CriticalSection guard1(Private::queueLock);
		Private::CriticalSection guard2(this->lock);
		Private::SourceStreamMap::Pair* p = sourceStreamMap.Find(this);
		if (p)
		{
			Private::SourceStream& ss = p->value;

			if (!ss.playing)
			{
				size_t oneSecLength = stream->Frequency() * stream->Channels() * (stream->Bits() / 8);
				size_t baseAlignment = stream->Channels() * (stream->Bits() / 8);

				size_t desiredLength = static_cast<size_t>(static_cast<double>(oneSecLength)* bufferingTime);
				size_t pp = desiredLength % baseAlignment;
				if (pp)
					desiredLength += baseAlignment - pp;

				ss.playing = true;
				ss.bufferSize = desiredLength;
				ss.loops = 1;
			}
			else
			{
				ss.request = DKFunction(this->source, &DKAudioSource::Play)->Invocation();
			}
			playbackCond.Signal();
			playerState = AudioState::StatePlaying;
		}
		else
		{
			DKLog("DKAudioPlayer(0x%x) is invalid.\n", this);
		}
	}
}

void DKAudioPlayer::Stop()
{
	if (this->source && this->stream)
	{
		Private::CriticalSection guard1(Private::queueLock);
		Private::CriticalSection guard2(this->lock);
		Private::SourceStreamMap::Pair* p = sourceStreamMap.Find(this);
		if (p)
		{
			Private::SourceStream& ss = p->value;

			ss.bufferSize = 0;
			ss.stream->SeekPcm(0);
			ss.request = DKFunction(this->source, &DKAudioSource::Stop)->Invocation();
			playerState = AudioState::StateStopped;
		}
		else
		{
			DKLog("DKAudioPlayer(0x%x) is invalid.\n", this);
		}
	}
}

void DKAudioPlayer::Pause()
{
	if (this->source && this->stream)
	{
		Private::CriticalSection guard1(Private::queueLock);
		Private::CriticalSection guard2(this->lock);
		Private::SourceStreamMap::Pair* p = sourceStreamMap.Find(this);
		if (p)
		{
			Private::SourceStream& ss = p->value;
			ss.bufferPos = 0;
			ss.request = DKFunction(this->source, &DKAudioSource::Pause)->Invocation();
			playerState = AudioState::StatePaused;
		}
		else
		{
			DKLog("DKAudioPlayer(0x%x) is invalid.\n", this);
		}
	}
}

int DKAudioPlayer::Channels() const
{
	if (stream)
		return stream->Channels();
	return 0;
}

int DKAudioPlayer::Bits() const
{
	if (stream)
		return stream->Bits();
	return 0;
}

int DKAudioPlayer::Frequency() const
{
	if (stream)
		return stream->Frequency();
	return 0;
}

double DKAudioPlayer::Duration() const
{
	return duration;
}

double DKAudioPlayer::TimePosition() const
{
	return timePosition;
}

DKAudioPlayer::AudioState DKAudioPlayer::State() const
{
	return playerState;
}

void DKAudioPlayer::UpdatePlaybackState(int ps, double tp)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	queuePlaybackState = ps;
	timePosition = tp;

	if (queuePlaybackState == QueueStatePlaybackStopped)
	{
		this->playerState = AudioState::StateStopped;
	}
}

void DKAudioPlayer::UpdateBufferState(int bs, double tp)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	queueBufferState = bs;
}

void DKAudioPlayer::ProcessStream(void *data, size_t size, double time)
{
	Private::CriticalSection guard(this->lock);

	// filtering PCM buffer. (do EQ or effects, etc.)
	if (filter)
		filter->Invoke(data, size, time);
}

DKAudioSource* DKAudioPlayer::AudioSource()
{
	return source;
}

const DKAudioSource* DKAudioPlayer::AudioSource() const
{
	return source;
}

void DKAudioPlayer::SetStreamFilter(StreamFilter* f)
{
	Private::CriticalSection guard(this->lock);

	filter = f;
}

void DKAudioPlayer::SetBufferingTime(double t)
{
	bufferingTime = Max(t, 0.05);
}

double DKAudioPlayer::BufferingTime() const
{
	return bufferingTime;
}
