//
//  File: DKTimer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"

namespace DKFoundation
{
	typedef int			DKTimeTick;		///< counter type default.
	typedef uint32_t	DKTimeTick32;	///< 32bit counter type.
	typedef uint64_t	DKTimeTick64;	///< 64bit counter type.

	/// @brief A high-resolution timer.
	///
	/// calculations
	///  - time = tick / frequency
	///  - tick = time * frequency
	///
	/// @note
	///  some bits could be lost while compute a real-number (double-precision) process,
	///  because of tick64 is 64 bits integer, but double has 52 bits fraction.
	///  (based on IEEE754)
	///
	///  DKTimer::Reset, DKTimer::Elapsed can produces good quality of result,
	///  it calculates time offset since Reset().
	class DKGL_API DKTimer
	{
	public:
		typedef DKTimeTick64 Tick; ///< using 64 bit counter.
		DKTimer();
		~DKTimer();

		/// reset timer, returns time elapsed since previous reset.
		double Reset();
		/// returns elapsed timer since time since last reset.
		double Elapsed() const;

		static Tick SystemTick();          ///< system tick
		static Tick SystemTickFrequency(); ///< tick frequency (a sec)
	private:
		Tick timeStamp;
	};
}
