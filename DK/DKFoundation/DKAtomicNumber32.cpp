//
//  File: DKAtomicNumber32.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKAtomicNumber32.h"

#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <libkern/OSAtomic.h>
#endif

using namespace DKFoundation;

DKAtomicNumber32::Value DKAtomicNumber32::Increment(void)
{
	Value prev;
#ifdef _WIN32
	prev = ::InterlockedIncrement((LONG*)&atomic);
	prev--;
#elif defined(__APPLE__) && defined(__MACH__)
	prev = ::OSAtomicIncrement32(&atomic);
	prev--;
#else
	prev = __sync_fetch_and_add(&atomic, 1);
#endif
	return prev;
}

DKAtomicNumber32::Value DKAtomicNumber32::Decrement(void)
{
	Value prev;
#ifdef _WIN32
	prev = ::InterlockedDecrement((LONG*)&atomic);
	prev++;
#elif defined(__APPLE__) && defined(__MACH__)
	prev = ::OSAtomicDecrement32(&atomic);
	prev++;
#else
	prev = __sync_fetch_and_sub(&atomic, 1);
#endif
	return prev;
}

DKAtomicNumber32::Value DKAtomicNumber32::Add(Value addend)
{
	Value prev;
#ifdef _WIN32
	prev = ::InterlockedExchangeAdd((LONG*)&atomic, addend);
#elif defined(__APPLE__) && defined(__MACH__)
	prev = ::OSAtomicAdd32(addend, &atomic);
	prev += addend;
#else
	prev = __sync_fetch_and_add(&atomic, addend);
#endif
	return prev;
}

DKAtomicNumber32::Value DKAtomicNumber32::Exchange(Value value)
{
	Value prev;
#ifdef _WIN32
	prev = ::InterlockedExchange((LONG*)&atomic, value);
#elif defined(__APPLE__) && defined(__MACH__)
	do 	{
		prev = atomic;
	} while (!::OSAtomicCompareAndSwap32(prev, value, &atomic));
#else
	do {
		prev = atomic;
	} while (__sync_val_compare_and_swap(&atomic, prev, value) != prev);
#endif
	return prev;
}

bool DKAtomicNumber32::CompareAndSet(Value comparand, Value value)
{
#ifdef _WIN32
	return ::InterlockedCompareExchange((LONG*)&atomic, value, comparand) == comparand;
#elif defined(__APPLE__) && defined(__MACH__)
	return ::OSAtomicCompareAndSwap32(comparand, value, &atomic);
#else
	return __sync_bool_compare_and_swap(&atomic, comparand, value);
#endif
}

DKAtomicNumber32::DKAtomicNumber32(Value initialValue)
	: atomic(initialValue)
{
}

DKAtomicNumber32::~DKAtomicNumber32(void)
{
}

DKAtomicNumber32& DKAtomicNumber32::operator = (Value value)
{
	Exchange(value);
	return *this;
}

DKAtomicNumber32& DKAtomicNumber32::operator += (Value value)
{
	Add(value);
	return *this;
}

DKAtomicNumber32::operator Value (void) const
{
	return (int)atomic;
}
