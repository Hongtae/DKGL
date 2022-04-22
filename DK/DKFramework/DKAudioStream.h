//
//  File: DKAudioStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	/// @brief interface for decode to PCM stream.
	///
	/// @note
	///   ogg-flac, ogg-vorbis, flac, wav(PCM) are built-in supported.\n
	///   You need to subclass to provde your own formatted stream.
	class DKGL_API DKAudioStream
	{
	public:
		typedef int64_t Position;
		enum FileType
		{
			FileTypeUnknown = 0,	///< Custom
			FileTypeOggVorbis,		///< built-in ogg-vorbis
			FileTypeOggFLAC,		///< built-in ogg-FLAC
			FileTypeFLAC,			///< built-in FLAC
			FileTypeWave,			///< built-in WAV(PCM)
		};

		static DKObject<DKAudioStream>	Create(DKStream* stream);
		static DKObject<DKAudioStream>	Create(const DKString& file);

		DKAudioStream(FileType t);
		virtual ~DKAudioStream();

		// media info.
		bool IsSeekable() const				{return seekable;}
		unsigned int SampleRate() const		{return sampleRate;} ///< sample rate
		unsigned int Channels() const		{return channels;}  ///< audio channels
		unsigned int Bits() const			{return bits;}      ///< format bits
		FileType MediaType() const			{return type;}

		////////////////////////////////////////////////////////////////////////////////
		// following functions should be overridden for decoding.
		virtual size_t Read(void*, size_t) = 0;
		// seeking media position (file, pcm, time)
		virtual Position SeekRaw(Position) = 0; ///< seek by stream pos
		virtual Position SeekPcm(Position) = 0; ///< seek by PCM
		virtual double SeekTime(double) = 0;    ///< seek by time
		// get media position
		virtual Position RawPos() const = 0; ///< stream position
		virtual Position PcmPos() const = 0; ///< PCM position (big size)
		virtual double TimePos() const = 0;  ///< time position
		// get media length
		virtual Position RawTotal() const = 0; ///< entire stream length
		virtual Position PcmTotal() const = 0; ///< entire PCM length (in bytes)
		virtual double TimeTotal() const = 0;  ///< entire length in time

	protected:
		void SetSeekable(bool s)					{seekable = s;}
		void SetSampleRate(unsigned int rate)		{sampleRate = rate;}
		void SetChannels(unsigned int c)			{channels = c;}
		void SetBits(unsigned int b)				{bits = b;}

	private:
		static FileType DetermineAudioType(char* data, size_t len);
		bool seekable;
		unsigned int sampleRate;
		unsigned int channels;
		unsigned int bits;
		const FileType type;
	};
}
