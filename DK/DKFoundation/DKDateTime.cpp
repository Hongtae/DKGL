//
//  File: DKDateTime.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <math.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef __ANDROID__
#include <time64.h>
#define timegm timegm64
#endif

#include "DKDateTime.h"
#include "DKString.h"
#include "DKSet.h"

#ifdef _WIN32
namespace DKFoundation
{
	namespace Private
	{
		namespace
		{
			inline void gettimeofday(struct timeval* p, void* tz)
			{
				union
				{
					int64_t ns100; // time since 1 Jan 1601 in 100ns units
					FILETIME ft;
				} timeUTC;

				GetSystemTimeAsFileTime(&timeUTC.ft);

				p->tv_usec = (long)((timeUTC.ns100 / 10LL) % 1000000LL );
				p->tv_sec =  (long)((timeUTC.ns100-(116444736000000000LL))/10000000LL);
			}
		}
	}
}
#endif

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKDateTime DKDateTime::Now(void)
{
	timeval tm;
	gettimeofday(&tm, NULL);
	return DKDateTime(static_cast<uint64_t>(tm.tv_sec), static_cast<uint32_t>(tm.tv_usec));
}

DKDateTime::DKDateTime(double d)
	: seconds(floor(d))
	, microseconds((d - floor(d))*1000000)
{
}

DKDateTime::DKDateTime(uint64_t s, uint32_t us)
	: seconds(0), microseconds(0)
{
	seconds = Max(s, uint64_t(0));
	microseconds = Min(us, uint32_t(999999));
}

DKDateTime::DKDateTime(int year, int month, int day, int hour, int min, int sec, int msec, bool utc)
	: seconds(0)
	, microseconds(0)
{
	DKDateTime dt;
	if (GetDateTime(dt, year, month, day, hour, min, sec, msec, utc))
	{
		this->seconds = dt.seconds;
		this->microseconds = dt.microseconds;
	}
}

DKDateTime::DKDateTime(int year, int month, int day, int hour, int min, int sec, int msec, int timezone)
	: seconds(0)
	, microseconds(0)
{
	DKDateTime dt;
	if (GetDateTime(dt, year, month, day, hour, min, sec, msec, timezone))
	{
		this->seconds = dt.seconds;
		this->microseconds = dt.microseconds;
	}
}

DKDateTime::DKDateTime(const DKString& iso8601)
	: seconds(0)
	, microseconds(0)
{
	DKDateTime dt;
	if (GetDateTime(dt, iso8601))
	{
		this->seconds = dt.seconds;
		this->microseconds = dt.microseconds;
	}
}

DKDateTime::DKDateTime(const DKDateTime& time)
	: seconds(time.seconds)
	, microseconds(time.microseconds)
{
}

DKDateTime::DKDateTime(void)
	: seconds(0)
	, microseconds(0)
{
}

DKDateTime::~DKDateTime(void)
{
}

void DKDateTime::GetComponent(Component& c) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *gmtime(&t);

	c.year = date.tm_year + 1900;
	c.month = date.tm_mon + 1;
	c.day = date.tm_mday;
	c.dayOfYear = date.tm_yday;
	c.dayOfWeek = date.tm_wday;
	c.hour = date.tm_hour;
	c.minute = date.tm_min;
	c.second = date.tm_sec;
	c.microsecond = microseconds;
}

void DKDateTime::GetLocalComponent(Component& c) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);

	c.year = date.tm_year + 1900;
	c.month = date.tm_mon + 1;
	c.day = date.tm_mday;
	c.dayOfYear = date.tm_yday;
	c.dayOfWeek = date.tm_wday;
	c.hour = date.tm_hour;
	c.minute = date.tm_min;
	c.second = date.tm_sec;
	c.microsecond = microseconds;
}

int DKDateTime::Year(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_year + 1900;
}

int DKDateTime::Month(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_mon + 1;
}

int DKDateTime::Day(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_mday;
}

int DKDateTime::DayOfYear(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_yday;
}

int DKDateTime::DayOfWeek(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_wday;
}

int DKDateTime::Hour(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_hour;
}

int DKDateTime::Minute(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_min;
}

int DKDateTime::Second(void) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);
	return date.tm_sec;
}

int DKDateTime::Microsecond(void) const
{
	return microseconds;
}

int64_t DKDateTime::DaysSinceEpoch(void) const
{
	return seconds / (60*60*24);
}

int64_t DKDateTime::HoursSinceEpoch(void) const
{
	return seconds / (60*60);
}

int64_t DKDateTime::MinutesSinceEpoch(void) const
{
	return seconds / 60;
}

int64_t DKDateTime::SecondsSinceEpoch(void) const
{
	return seconds;
}

long DKDateTime::TimezoneOffset(void)
{
	time_t utc = time(0);
	struct tm date = *gmtime(&utc);
	const time_t local = mktime(&date);
	return static_cast<long>(static_cast<int64_t>(utc) - static_cast<int64_t>(local));
}

DKDateTime DKDateTime::operator + (double d) const
{
	return DKDateTime(IntervalSinceEpoch() + d);
}

DKDateTime& DKDateTime::operator += (double d)
{
	return this->operator = (this->operator + (d));
}

DKDateTime DKDateTime::operator - (double d) const
{
	return DKDateTime(IntervalSinceEpoch() - d);
}

DKDateTime& DKDateTime::operator -= (double d)
{
	return this->operator = (this->operator - (d));
}

DKDateTime& DKDateTime::operator = (const DKDateTime& t)
{
	this->seconds = t.seconds;
	this->microseconds = t.microseconds;
	return *this;
}

bool DKDateTime::operator > (const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds > t.microseconds;
	return this->seconds > t.seconds;
}

bool DKDateTime::operator >= (const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds >= t.microseconds;
	return this->seconds >= t.seconds;
}

bool DKDateTime::operator < (const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds < t.microseconds;
	return this->seconds < t.seconds;
}

bool DKDateTime::operator <= (const DKDateTime& t) const	
{
	if (this->seconds == t.seconds)
		return this->microseconds <= t.microseconds;
	return this->seconds <= t.seconds;
}

bool DKDateTime::operator == (const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds == t.microseconds;
	return false;
}

bool DKDateTime::operator != (const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds != t.microseconds;
	return true;
}

double DKDateTime::Interval(const DKDateTime& t) const
{
	return this->IntervalSinceEpoch() - t.IntervalSinceEpoch();
}

double DKDateTime::IntervalSinceEpoch(void) const
{
	return static_cast<double>(this->seconds) + (static_cast<double>(this->microseconds) * 0.000001);
}

DKString DKDateTime::Format(const DKString& fmt) const
{
	const time_t t = (time_t)seconds;
	struct tm date = *localtime(&t);

	wchar_t buff[1024];
	buff[0] = 0;
	wcsftime(buff, 1024, (const wchar_t*)fmt, &date);

	return DKString(buff);
}

DKString DKDateTime::Format(DateFormat df, TimeFormat tf) const
{
	DKString dateStr = L"";
	DKString timeStr = L"";

	switch (df)
	{
	case DateFormatNormal:
		dateStr = Format(L"%Y-%m-%d");
		break;
	case DateFormatWithWeekday:
		dateStr = Format(L"%Y-%m-%d %A");
		break;
	case DateFormatWithWeekdayShort:
		dateStr = Format(L"%Y-%m-%d %a");
		break;
	}

	DKString timezone = L"";
	bool localizedTimezone = true;
	if (localizedTimezone)
	{
		timezone = Format(L"%z");
	}
	else
	{
		long offset = TimezoneOffset();
		int minutes = labs(offset) / 60;
		int hours = minutes / 60;
		minutes = minutes % 60;
		if (offset < 0)
		{
			timezone = DKString::Format("-%02d:%02d", hours, minutes);
		}
		else
		{
			timezone = DKString::Format("+%02d:%02d", hours, minutes);
		}
	}

	switch (tf)
	{
	case TimeFormat12Hour:
		timeStr = Format(L"%I:%M:%S %p");
		break;
	case TimeFormat12HourWithMillisecond:
		timeStr = DKString::Format("%ls.%03u %ls", (const wchar_t*)Format(L"%I:%M:%S"), (this->microseconds / 1000), (const wchar_t*)Format(L"%p"));
		break;
	case TimeFormat12HourWithMicrosecond:
		timeStr = DKString::Format("%ls.%06u %ls", (const wchar_t*)Format(L"%I:%M:%S"), this->microseconds, (const wchar_t*)Format(L"%p"));
		break;
	case TimeFormat12HourWithTimezone:
		timeStr = DKString::Format("%ls %ls", (const wchar_t*)Format(L"%I:%M:%S %p "), (const wchar_t*)timezone);
		break;
	case TimeFormat12HourWithMillisecondTimezone:
		timeStr = DKString::Format("%ls.%03u %ls %ls", (const wchar_t*)Format(L"%I:%M:%S"), (this->microseconds / 1000), (const wchar_t*)Format(L"%p"), (const wchar_t*)timezone);
		break;
	case TimeFormat12HourWithMicrosecondTimezone:
		timeStr = DKString::Format("%ls.%06u %ls %ls", (const wchar_t*)Format(L"%I:%M:%S"), this->microseconds, (const wchar_t*)Format(L"%p"), (const wchar_t*)timezone);
		break;
	case TimeFormat24Hour:
		timeStr = Format(L"%H:%M:%S");
		break;
	case TimeFormat24HourWithMillisecond:
		timeStr = DKString::Format("%ls.%03u", (const wchar_t*)Format(L"%H:%M:%S"), (this->microseconds / 1000));
		break;
	case TimeFormat24HourWithMicrosecond:
		timeStr = DKString::Format("%ls.%06u", (const wchar_t*)Format(L"%H:%M:%S"), this->microseconds);
		break;
	case TimeFormat24HourWithTimezone:
		timeStr = DKString::Format("%ls %ls", (const wchar_t*)Format(L"%H:%M:%S"), (const wchar_t*)timezone);
		break;
	case TimeFormat24HourWithMillisecondTimezone:
		timeStr = DKString::Format("%ls.%03u %ls", (const wchar_t*)Format(L"%H:%M:%S"), (this->microseconds / 1000), (const wchar_t*)timezone);
		break;
	case TimeFormat24HourWithMicrosecondTimezone:
		timeStr = DKString::Format("%ls.%06u %ls", (const wchar_t*)Format(L"%H:%M:%S"), this->microseconds, (const wchar_t*)timezone);
		break;
	}

	size_t dsLen = dateStr.Length();
	size_t tsLen = timeStr.Length();
	if (dsLen > 0 && tsLen > 0)
		return DKString::Format("%ls, %ls", (const wchar_t*)dateStr, (const wchar_t*)timeStr);
	if (dsLen > 0)
		return dateStr;
	return timeStr;
}

DKString DKDateTime::FormatISO8601(bool utc) const
{
	DKString ret = L"";
	char buff[1024];
	if (utc)
	{
		const time_t t = (time_t)seconds;
		struct tm date = *gmtime(&t);
		strftime(buff, 1024, "%Y-%m-%dT%H:%M:%S", &date);

		if (this->microseconds > 0)
			ret = DKString::Format("%s.%06uZ", buff, (unsigned int)Clamp(this->microseconds, 0, 999999));
		else
			ret = DKString::Format("%sZ", buff);
	}
	else
	{
		long offset = TimezoneOffset();
		int minutes = labs(offset) / 60;
		int hours = minutes / 60;
		minutes = minutes % 60;

		const time_t t = (time_t)seconds;
		struct tm date = *localtime(&t);
		strftime(buff, 1024, "%Y-%m-%dT%H:%M:%S", &date);

		if (this->microseconds > 0)
			ret = DKString::Format("%s.%06u", buff, (unsigned int)Clamp(this->microseconds, 0, 999999));
		else
			ret.SetValue(buff);

		if (offset < 0)
		{
			ret.Append(DKString::Format("-%02d:%02d", hours, minutes));
		}
		else
		{
			ret.Append(DKString::Format("+%02d:%02d", hours, minutes));
		}
	}
	return ret;
}

bool DKDateTime::GetDateTime(DKDateTime& dtOut, int year, int month, int day, int hour, int min, int sec, int msec, bool utc)
{
	struct tm t;
	memset(&t, 0, sizeof(tm));
	t.tm_year = year - 1900;
	t.tm_mon = Clamp(month, 1, 12) - 1;
	t.tm_mday = Clamp(day, 1, 31);
	t.tm_hour = Clamp(hour, 0, 23);
	t.tm_min = Clamp(min, 0, 59);
	t.tm_sec = Clamp(sec, 0, 59);

	time_t seconds = 0;
	if (utc)
	{
#ifdef _WIN32
		seconds = _mkgmtime(&t);	
#else
		seconds = timegm(&t);
#endif
	}
	else
	{
		seconds = mktime(&t);
	}

	if (seconds >= 0)
	{
		dtOut.seconds = seconds;
		dtOut.microseconds = Clamp(msec, 0, 999999);
		return true;
	}
	return false;
}

bool DKDateTime::GetDateTime(DKDateTime& dtOut, int year, int month, int day, int hour, int min, int sec, int msec, int timezone)
{
	DKDateTime dt;
	if (GetDateTime(dt, year, month, day, hour, min, sec, msec, true))
	{
		dtOut.seconds = dt.seconds + timezone;
		dtOut.microseconds = dt.microseconds;
	}
	return false;
}

bool DKDateTime::GetDateTime(DKDateTime& dtOut, int year, int dayOfYear, int hour, int min, int sec, int msec, bool utc)
{
	struct tm t;
	memset(&t, 0, sizeof(tm));
	t.tm_year = year - 1900;
	t.tm_yday = Clamp(dayOfYear, 0, 365);
	t.tm_hour = Clamp(hour, 0, 23);
	t.tm_min = Clamp(min, 0, 59);
	t.tm_sec = Clamp(sec, 0, 59);
	
	time_t seconds = 0;
	if (utc)
	{
#ifdef _WIN32
		seconds = _mkgmtime(&t);	
#else
		seconds = timegm(&t);
#endif
	}
	else
	{
		seconds = mktime(&t);
	}
	
	if (seconds >= 0)
	{
		dtOut.seconds = seconds;
		dtOut.microseconds = Clamp(msec, 0, 999999);
		return true;
	}
	return false;
}

bool DKDateTime::GetDateTime(DKDateTime& dtOut, const DKString& str)
{
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
	//    On this implementation, You should specify date. becouse it is
	//    date-time object. (Can not be used with time only)
	//    Will not handles BC. (Year >= 0)

	DKString dateString = L"";
	DKString timeString = L"";
	bool extFormat = false;

	if (true)
	{
		DKString inputStr = DKString(str).TrimWhitespaces().UppercaseString();
		const wchar_t* inputString = inputStr;

		long ws = inputStr.FindWhitespaceCharacter(0);
		if (ws >= 0)
			inputStr = inputStr.Left(ws);

		size_t length = inputStr.Length();

		if (length < 7)
			return false;

		if (inputString[4] == L'-')
			extFormat = true;

		long t = inputStr.Find(L'T', 0);

		if (t >= 0)
		{
			dateString = inputStr.Left(t);
			timeString = inputStr.Right(t+1);
		}
		else
		{
			dateString = inputStr;
		}
	}

	struct {
		bool operator () (wchar_t c) const {return c >= L'0' && c <= L'9';}
	} isNumber;
	struct {
		bool operator () (wchar_t c) const {return c == L'-';}
	} isHyphen;
	struct {
		bool operator () (wchar_t c) const {return c == L':';}
	} isColon;

	// Dont use DKString::ToInteger, DKString::ToUnsignedInteger.
	// number strings leading by '0', will be converted as octal number.
	// format should be decimal, using below function object.
	struct {
		int operator () (const DKString& str) const
		{
			const wchar_t* s = str;
			size_t len = str.Length();
			int result = 0;
			int sign = 1;

			size_t i = 0;
			if (s[0] == L'+')
			{
				sign = 1;
				i++;
			}
			else if (s[0] == L'-')
			{
				sign = -1;
				i++;
			}
			for ( ; i < len; ++i)
			{
				if (s[i] >= L'0' && s[i] <= L'9')
				{
					result *= 10;
					result += s[i] - L'0';
				}
				else
					break;
			}
			return result * sign;
		}
	} decimalStringToInteger;

	// week dates of ISO-8601 (YYYY-Www, YYYYWww, YYYY-Www-D, YYYYWwwD)
	// a week starting with Monday, ending with Sunday.
	struct
	{
		int operator () (int year) const
		{
			struct tm t;
			memset(&t, 0, sizeof(tm));
			t.tm_year = year - 1900;
			t.tm_yday = 0;

#ifdef _WIN32
			time_t secs = _mkgmtime(&t);	
#else
			time_t secs = timegm(&t);
#endif
			t = *gmtime(&secs);

			// Week number of Jan.1 is if Jan.1 is...
			//     Mon, Tues, Wed, Thur : week number is 01.
			//     Fri, Sat, Sun : week number is 52 or 53 of last year.
			if (t.tm_wday > 0 && t.tm_wday < 5)
			{
				return -(t.tm_wday - 1);		// week01
			}
			if (t.tm_wday == 5) // Jan.1 is Friday.
				return 3;
			if (t.tm_wday == 6) // Jan.1 is Saturday.
				return 2;
			return 1;		    // Jan.1 is Sunday.
		}
	} weekdateOffset;

	int year = 0;
	int month = 0;
	int day = 0;
	int daysOfYear = 0;
	bool dateByCalendar = true;
	bool validDate = false;

	const wchar_t* dateStr = dateString;
	size_t dateStrLen = dateString.Length();

	if (isNumber(dateStr[0]) && isNumber(dateStr[1]) && isNumber(dateStr[2]) && isNumber(dateStr[3]))
	{
		year = decimalStringToInteger(dateString.Left(4));

		if (extFormat)
		{
			if (isHyphen(dateStr[4]) && isNumber(dateStr[5]) && isNumber(dateStr[6]) && isNumber(dateStr[7]))		// YYYY-DDD
			{
				if (dateStrLen == 8)
				{
					daysOfYear = decimalStringToInteger(dateString.Mid(5, 3));
					if (daysOfYear >= 1 && daysOfYear <= 366)
					{
						dateByCalendar = false;
						validDate = true;
					}
				}					
			}
			else if (isHyphen(dateStr[4]) && isNumber(dateStr[5]) && isNumber(dateStr[6]))		// YYYY-MM ??
			{
				if (dateStrLen == 7)		// YYYY-MM
				{
					month = decimalStringToInteger(dateString.Mid(5, 2));
					day = 1;
				}
				else if (dateStrLen == 10 && isHyphen(dateStr[7]) && isNumber(dateStr[8]) && isNumber(dateStr[9]))	// YYYY-MM-DD
				{
					month = decimalStringToInteger(dateString.Mid(5, 2));
					day = decimalStringToInteger(dateString.Mid(8, 2));
				}
				if (month >= 1 && month <= 12 && day >= 1 && day <= 31)
				{
					dateByCalendar = true;
					validDate = true;
				}
			}
			else if (isHyphen(dateStr[4]) && dateStr[5] == L'W' && isNumber(dateStr[6]) && isNumber(dateStr[7]))		// YYYY-Www ??
			{
				int weeks = 0;
				int weekday = 0;
				
				if (dateStrLen == 8)	// YYYY-Www
				{
					weeks = decimalStringToInteger(dateString.Mid(6, 2));
					weekday = 1;
				}
				else if (dateStrLen == 10 && isHyphen(dateStr[8]) && isNumber(dateStr[9]))		// YYYY-Www-D
				{
					weeks = decimalStringToInteger(dateString.Mid(6, 2));
					weekday = dateStr[9] - L'0';
				}
				if (weeks >= 1 && weeks <= 53 && weekday >= 1 && weekday <= 7)
				{
					daysOfYear = (weeks - 1) * 7 + (weekday-1) + weekdateOffset(year);
					dateByCalendar = false;
					validDate = true;
				}
			}
		}
		else
		{
			if (isNumber(dateStr[4]) && isNumber(dateStr[5]) && isNumber(dateStr[6]) && isNumber(dateStr[7]))	// YYYYMMDD
			{
				month = decimalStringToInteger(dateString.Mid(4, 2));
				day = decimalStringToInteger(dateString.Mid(6, 2));
				
				if (month >= 1 && month <= 12 && day >= 1 && day <= 31)
				{
					dateByCalendar = true;
					validDate = true;
				}				
			}
			else if (isNumber(dateStr[4]) && isNumber(dateStr[5]) && isNumber(dateStr[6]))		// YYYYDDD
			{
				if (dateStrLen == 7)
				{
					daysOfYear = decimalStringToInteger(dateString.Mid(4, 3));
					if (daysOfYear >= 1 && daysOfYear <= 366)
					{
						dateByCalendar = false;
						validDate = true;
					}
				}			
			}
			else if (dateStr[4] == L'W' && isNumber(dateStr[5]) && isNumber(dateStr[6]))		// YYYYWww ??
			{
				int weeks = 0;
				int weekday = 0;

				if (dateStrLen == 7)	// YYYYWww
				{
					weeks = decimalStringToInteger(dateString.Mid(5, 2));
					weekday = 1;
				}
				else if (dateStrLen == 8 && isNumber(dateStr[7]))	// YYYYWwwD
				{
					weeks = decimalStringToInteger(dateString.Mid(5, 2));
					weekday = dateStr[7] - L'0';
				}				
				if (weeks >= 1 && weeks <= 53 && weekday >= 1 && weekday <= 7)
				{
					daysOfYear = (weeks - 1) * 7 + (weekday-1) + weekdateOffset(year);
					dateByCalendar = false;
					validDate = true;
				}				
			}
		}
	}

	if (validDate)
	{
		int32_t microseconds = 0;
		int64_t secondsSinceEpoch = 0;

		if (dateByCalendar)
		{
			secondsSinceEpoch = DKDateTime(year, month, day, 0, 0, 0, 0, true).seconds;
		}
		else
		{
			secondsSinceEpoch = DKDateTime(year, 1, 1, 0, 0, 0, 0, true).seconds + ((daysOfYear-1) * (60*60*24));
		}
		
		size_t timeStrLen = timeString.Length();
		if (timeStrLen > 0)
		{
			DKString timezoneString = L"";
			// getting timezone from 'Z', '+', '-'.
			long tz = timeString.FindAnyCharactersInSet(DKString::CharacterSet(L"Z+-", 3), 0);
			if (tz >= 0)
			{
				timezoneString = timeString.Right(tz);
				timeString = timeString.Left(tz);
				timeStrLen = tz;
			}				
			
			const wchar_t* timeStr = timeString;
			long seconds = 0;
			
			if (extFormat)
			{
				int hours = 0;
				int minutes = 0;
				size_t pos = 0;
				if (isNumber(timeStr[0]) && isNumber(timeStr[1]))					// HH
				{
					hours = decimalStringToInteger(timeString.Mid(0, 2));
					pos = 2;
					if (isColon(timeStr[2]) && isNumber(timeStr[3]) && isNumber(timeStr[4]))		// HH:MM
					{
						minutes = decimalStringToInteger(timeString.Mid(3, 2));
						pos = 5;
						if (isColon(timeStr[5]) && isNumber(timeStr[6]) && isNumber(timeStr[7]))	// HH:MM:SS
						{
							seconds = decimalStringToInteger(timeString.Mid(6, 2));
							pos = 8;
						}
						else if (timeStr[5])	// not null?
							return false;
					}
					else if (timeStr[2])		// not null?
						return false;
				}
				else
				{
					return false;
				}
				
				if (hours >= 0 && hours <= 24 && minutes >= 0 && minutes <= 59 && seconds >= 0 && seconds <= 60)
				{
					minutes += hours * 60;
					seconds += minutes * 60;
				}
				else
				{
					return false;
				}
				
				if (timeStr[pos] == L',' || timeStr[pos] == L'.')	// fraction
				{
					for (size_t i = 1; (pos + i) < timeStrLen; ++i)
					{
						if (!isNumber(timeStr[ (pos+i) ]))
						{
							return false;
							break;
						}
					}
					if (pos + 1 < timeStrLen)
					{
						double fraction = DKString("0.").Append(timeString.Right(pos+1)).ToRealNumber();
						if (pos == 2)
							fraction = fraction * 3600;
						else if (pos == 5)
							fraction = fraction * 60;

						seconds += floor(fraction);
						microseconds = static_cast<long>(( fraction - floor(fraction) ) * 1000000L );
					}
				}
				else if (timeStr[pos])		// not null?
					return false;
			}
			else
			{
				int hours = 0;
				int minutes = 0;
				size_t pos = 0;
				if (isNumber(timeStr[0]) && isNumber(timeStr[1]))			// HH
				{
					hours = decimalStringToInteger(timeString.Mid(0, 2));
					pos = 2;
					if (isNumber(timeStr[2]) && isNumber(timeStr[3]))		// HHMM
					{
						minutes = decimalStringToInteger(timeString.Mid(2, 2));
						pos = 4;
						if (isNumber(timeStr[4]) && isNumber(timeStr[5]))	// HHMMSS
						{
							seconds = decimalStringToInteger(timeString.Mid(4, 2));
							pos = 6;
						}
						else if (timeStr[4])	// not null?
							return false;
					}
					else if (timeStr[2])		// not null?
						return false;
				}
				else
				{
					return false;
				}
				
				if (hours >= 0 && hours <= 24 && minutes >= 0 && minutes <= 59 && seconds >= 0 && seconds <= 60)
				{
					minutes += hours * 60;
					seconds += minutes * 60;
				}
				else
				{
					return false;
				}
				
				if (timeStr[pos] == L',' || timeStr[pos] == L'.')	// fraction
				{
					for (size_t i = 1; (pos + i) < timeStrLen; ++i)
					{
						if (!isNumber(timeStr[ (pos+i) ]))
						{
							return false;
							break;
						}
					}
					if (pos + 1 < timeStrLen)
					{
						double fraction = DKString("0.").Append(timeString.Right(pos+1)).ToRealNumber();
						if (pos == 2)
							fraction = fraction * 3600;
						else if (pos == 4)
							fraction = fraction * 60;

						seconds += floor(fraction);
						microseconds = static_cast<long>(( fraction - floor(fraction) ) * 1000000L );
					}
				}
				else if (timeStr[pos])		// not null?
					return false;				
			}			
			
			bool hasTimezone = false;
			bool timezoneUTC = false;
			long timezoneOffset = 0;
			
			size_t timezoneStrLen = timezoneString.Length();
			if (timezoneStrLen > 0)
			{
				const wchar_t* timezoneStr = timezoneString;
				
				if (timezoneString.Compare(L"Z") == 0)
				{
					timezoneUTC = true;
				}
				else if (timezoneString.Compare(L"-00:00") == 0) // -00:00 is local time. (RFC-3339)
				{
					hasTimezone = false;
				}
				else if (timezoneStr[0] == L'+' || timezoneStr[0] == L'-')
				{
					int hh = 0;
					int mm = 0;
					hasTimezone = true;

					if (isNumber(timezoneStr[1]) && isNumber(timezoneStr[2]))	// HH
					{
						hh = decimalStringToInteger(timezoneString.Mid(1, 2));
						if (timezoneStrLen == 5 && isNumber(timezoneStr[3]) && isNumber(timezoneStr[4]))	// HHMM
							mm = decimalStringToInteger(timezoneString.Mid(3, 4));
						else if (timezoneStrLen == 6 && isColon(timezoneStr[3]) && isNumber(timezoneStr[4]) && isNumber(timezoneStr[5]))	// HH:MM
							mm = decimalStringToInteger(timezoneString.Mid(4, 5));
						else if (timezoneStr[3])
							return false;							
					}
					else
						return false;
					
					timezoneOffset = ((hh * 60) + mm) * 60;
					if (timezoneStr[0] == L'-')
						timezoneOffset *= -1;
				}
			}
			
			if (timezoneUTC)
			{
				secondsSinceEpoch += seconds;
			}
			else
			{
				secondsSinceEpoch += seconds;
				if (hasTimezone)
					secondsSinceEpoch -= timezoneOffset;
				else
					secondsSinceEpoch -= DKDateTime::Now().TimezoneOffset();
			}
		}
		
		dtOut = DKDateTime(secondsSinceEpoch, microseconds);
		return true;
	}
	return false;
}

int DKDateTime::Compare(const DKDateTime& t) const
{
	if (this->seconds == t.seconds)
		return this->microseconds - t.microseconds;
	return this->seconds - t.seconds;
}

int DKDateTime::CompareDate(const DKDateTime& t) const
{
	return this->DaysSinceEpoch() - t.DaysSinceEpoch();
}

int DKDateTime::CompareDateTimeWithoutSeconds(const DKDateTime& t) const
{
	return this->MinutesSinceEpoch() - t.MinutesSinceEpoch();
}

int DKDateTime::CompareDateTimeWithoutMicroseconds(const DKDateTime& t) const
{
	return this->seconds - t.seconds;
}


