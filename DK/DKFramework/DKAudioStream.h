//
//  File: DKAudioStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKAudioStream
// An abstract class, interface for decode to PCM stream.
// You need to subclass to provde your own formatted stream.
//
// Note:
//   ogg-flac, ogg-vorbis, flac, wav(PCM) are supported currently.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKAudioStream
	{
	public:
		typedef long long Position;
		enum FileType
		{
			FileTypeUnknown = 0,
			FileTypeOggVorbis,
			FileTypeOggFLAC,
			FileTypeFLAC,
			FileTypeWave,
		};

		static DKFoundation::DKObject<DKAudioStream>	Create(DKFoundation::DKStream* stream);
		static DKFoundation::DKObject<DKAudioStream>	Create(const DKFoundation::DKString& file);

		DKAudioStream(FileType t);
		virtual ~DKAudioStream(void);

		// media info.
		bool IsSeekable(void) const				{return seekable;}
		unsigned int Frequency(void) const		{return frequency;} // bitrate
		unsigned int Channels(void) const		{return channels;}  // audio channels
		unsigned int Bits(void) const			{return bits;}      // format bits
		FileType MediaType(void) const			{return type;}

		////////////////////////////////////////////////////////////////////////////////
		// following functions should be overridden for decoding.
		virtual size_t Read(void*, size_t) = 0;
		// seeking media position (file, pcm, time)
		virtual Position SeekRaw(Position) = 0; // seek by stream pos
		virtual Position SeekPcm(Position) = 0; // seek by PCM
		virtual double SeekTime(double) = 0;    // seek by time
		// get media position
		virtual Position RawPos(void) const = 0; // stream position
		virtual Position PcmPos(void) const = 0; // PCM position (big size)
		virtual double TimePos(void) const = 0;  // time position
		// get media length
		virtual Position RawTotal(void) const = 0; // entire stream length
		virtual Position PcmTotal(void) const = 0; // entire PCM length (in bytes)
		virtual double TimeTotal(void) const = 0;  // entire length in time

	protected:
		void SetSeekable(bool s)					{seekable = s;}
		void SetFrequency(unsigned int freq)		{frequency = freq;}
		void SetChannels(unsigned int c)			{channels = c;}
		void SetBits(unsigned int b)				{bits = b;}

	private:
		static FileType DetermineAudioType(char* data, size_t len);
		bool seekable;
		unsigned int frequency;
		unsigned int channels;
		unsigned int bits;
		const FileType type;
	};
}
