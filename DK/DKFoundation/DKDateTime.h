//
//  File: DKDateTime.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

////////////////////////////////////////////////////////////////////////////////
// DKDateTime
// Calculate date and time based on UTC timezone.
// You can create object from ISO-8601 formatted string.
//
// ISO-8601 formats
// date format (calendar dates):
//            YYYY-MM-DD, YYYYMMDD, YYYY-MM (not YYYYMM)
//            MM is from 01 to 12, DD is from 01 to 31
// date format (week dates):
//            YYYY-Www, YYYYWww, YYYY-Www-D, YYYYWwwD (Www is W + week-no.)
//            Www is from W01 to W53, D is from 1 to 7
//            ex: 2012-W14-7, 7days from 14th weeks of 2012
// date format (ordinal dates):
//            YYYY-DDD, YYYYDDD
//            DDD is from 001 to 365
//            ex: 1981-04-05 = 1981-095
// time format:
//            hh:mm:ss, hhmmss, hh:mm, hhmm, hh
//            hh is from 00 to 24, mm is from 00 to 59, ss is from 00 to 60
//            (60 is a leap second by Caesium atomic clocks.)
// time with timezone format:
//            appended Z, ±hh:mm, ±hhmm, ±hh to time format. (Z for UTC)
//
// for date and time combined:
//            described as <date>T<time>
//            ex: 2007-04-05T14:30
//
// Note:
//    Unix Epoch is 1970-01-01 00:00:00 UTC (1970-01-01T00:00:00Z ISO 8601)
//    You should specify date with ISO-8601 format.
//    This implementation will not handles BC. (Year >= 0)
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKDateTime
	{
	public:
		enum DateFormat
		{
			DateFormatNone = 0,
			DateFormatNormal,
			DateFormatWithWeekday,
			DateFormatWithWeekdayShort,
		};
		enum TimeFormat
		{
			TimeFormatNone = 0,
			TimeFormat12Hour,
			TimeFormat12HourWithMillisecond,
			TimeFormat12HourWithMicrosecond,
			TimeFormat12HourWithTimezone,
			TimeFormat12HourWithMillisecondTimezone,
			TimeFormat12HourWithMicrosecondTimezone,
			TimeFormat24Hour,
			TimeFormat24HourWithMillisecond,
			TimeFormat24HourWithMicrosecond,
			TimeFormat24HourWithTimezone,
			TimeFormat24HourWithMillisecondTimezone,
			TimeFormat24HourWithMicrosecondTimezone,
		};
		struct Component
		{
			unsigned short year;
			unsigned char month;
			unsigned char day;
			unsigned char hour;
			unsigned char minute;
			unsigned char second;
			unsigned int microsecond;
			unsigned char dayOfWeek;
			unsigned int dayOfYear;
		};

		static DKDateTime Now(void);
		explicit DKDateTime(double d);
		explicit DKDateTime(uint64_t seconds, uint32_t microseconds);	// UTC (not local-time)
		explicit DKDateTime(int year, int month, int day, int hour, int min, int sec, int msec, bool utc = false);
		explicit DKDateTime(int year, int month, int day, int hour, int min, int sec, int msec, int timezone); // timezone is second unit.
		explicit DKDateTime(const DKString& iso8601);
		DKDateTime(const DKDateTime& time);
		DKDateTime(void);
		~DKDateTime(void);

		void GetComponent(Component& c) const;			// UTC
		void GetLocalComponent(Component& c) const;		// local-time

		int Year(void) const;
		int Month(void) const;
		int Day(void) const;
		int DayOfYear(void) const;
		int DayOfWeek(void) const;
		int Hour(void) const;
		int Minute(void) const;
		int Second(void) const;
		int Microsecond(void) const;

		int64_t DaysSinceEpoch(void) const;
		int64_t HoursSinceEpoch(void) const;
		int64_t MinutesSinceEpoch(void) const;
		int64_t SecondsSinceEpoch(void) const;
		static long TimezoneOffset(void);			// returns second offsets with GMT

		DKDateTime operator + (double d) const;
		DKDateTime& operator += (double d);
		DKDateTime operator - (double d) const;
		DKDateTime& operator -= (double d);
		DKDateTime& operator = (const DKDateTime& t);

		// calculate time interval, can be negative if t is greater.
		// Use DKTimer for high-resolution timer.
		double Interval(const DKDateTime& t) const;
		double IntervalSinceEpoch(void) const;

		// same as strftime (wcsftime) (can be different result generated by platform.)
		DKString Format(const DKString& fmt) const;
		DKString Format(DateFormat df, TimeFormat tf) const;
		DKString FormatISO8601(bool utc = false) const;
		static bool GetDateTime(DKDateTime& dtOut, int year, int month, int day, int hour, int min, int sec, int msec, bool utc);
		static bool GetDateTime(DKDateTime& dtOut, int year, int month, int day, int hour, int min, int sec, int msec, int timezone); // timezone is second unit.
		static bool GetDateTime(DKDateTime& dtOut, int year, int dayOfYear, int hour, int min, int sec, int msec, bool utc);
		// read from ISO-8601 string.
		static bool GetDateTime(DKDateTime& dtOut, const DKString& iso8601str);

		int Compare(const DKDateTime& t) const;
		int CompareDate(const DKDateTime& t) const;
		int CompareDateTimeWithoutSeconds(const DKDateTime& t) const;
		int CompareDateTimeWithoutMicroseconds(const DKDateTime& t) const;

		bool operator > (const DKDateTime& t) const;
		bool operator >= (const DKDateTime& t) const;
		bool operator < (const DKDateTime& t) const;
		bool operator <= (const DKDateTime& t) const;
		bool operator == (const DKDateTime& t) const;
		bool operator != (const DKDateTime& t) const;

	private:
		uint64_t seconds;		// second unit. (have 68years cycles in 32bit)
		uint32_t microseconds;	// micro-second unit. (1 ~ 1,000,000)
	};
}
