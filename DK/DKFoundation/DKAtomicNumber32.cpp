//
//  File: DKAtomicNumber32.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <libkern/OSAtomic.h>
#endif

#include "DKAtomicNumber32.h"

using namespace DKFoundation;

DKAtomicNumber32::Value DKAtomicNumber32::Increment()
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedIncrement((LONG*)&atomic);
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicIncrement32(&atomic);
#else
    value = __sync_add_and_fetch(&atomic, 1);
#endif
	return value;
}

DKAtomicNumber32::Value DKAtomicNumber32::Decrement()
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedDecrement((LONG*)&atomic);
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicDecrement32(&atomic);
#else
    value = __sync_sub_and_fetch(&atomic, 1);
#endif
	return value;
}

DKAtomicNumber32::Value DKAtomicNumber32::Add(Value addend)
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedExchangeAdd((LONG*)&atomic, addend);
    value += addend;
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicAdd32(addend, &atomic);
#else
    value = __sync_fetch_and_add(&atomic, addend);
#endif
	return value;
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

DKAtomicNumber32::~DKAtomicNumber32()
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

DKAtomicNumber32::operator Value () const
{
	return (int)atomic;
}
