//
//  File: DKAtomicNumber64.cpp
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

#include "DKAtomicNumber64.h"

using namespace DKFoundation;

DKAtomicNumber64::Value DKAtomicNumber64::Increment()
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedIncrement64((LONGLONG*)&atomic);
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicIncrement64(&atomic);
#else
    value = __sync_add_and_fetch(&atomic, 1);
#endif
	return value;
}

DKAtomicNumber64::Value DKAtomicNumber64::Decrement()
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedDecrement64((LONGLONG*)&atomic);
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicDecrement64(&atomic);
#else
    value = __sync_sub_and_fetch(&atomic, 1);
#endif
	return value;
}

DKAtomicNumber64::Value DKAtomicNumber64::Add(Value addend)
{
	Value value;
#ifdef _WIN32
    value = ::InterlockedExchangeAdd64((LONGLONG*)&atomic, addend);
#elif defined(__APPLE__) && defined(__MACH__)
    value = ::OSAtomicAdd64(addend, &atomic);
#else
    value = __sync_add_and_fetch(&atomic, addend);
#endif
	return value;
}

DKAtomicNumber64::Value DKAtomicNumber64::Exchange(Value value)
{
	Value prev;
#ifdef _WIN32
	prev = ::InterlockedExchange64((LONGLONG*)&atomic, value);
#elif defined(__APPLE__) && defined(__MACH__)
	do 	{
		prev = atomic;
	} while (!::OSAtomicCompareAndSwap64(prev, value, &atomic));
#else
	do {
		prev = atomic;
	} while (__sync_val_compare_and_swap(&atomic, prev, value) != prev);
#endif
	return prev;
}

bool DKAtomicNumber64::CompareAndSet(Value comparand, Value value)
{
#ifdef _WIN32
	return ::InterlockedCompareExchange64((LONGLONG*)&atomic, value, comparand) == comparand;
#elif defined(__APPLE__) && defined(__MACH__)
	return ::OSAtomicCompareAndSwap64(comparand, value, &atomic);
#else
	return __sync_bool_compare_and_swap(&atomic, comparand, value);
#endif
}

DKAtomicNumber64::DKAtomicNumber64(Value initialValue)
	: atomic(initialValue)
{
}

DKAtomicNumber64::~DKAtomicNumber64()
{
}

DKAtomicNumber64& DKAtomicNumber64::operator = (Value value)
{
	Exchange(value);
	return *this;
}

DKAtomicNumber64& DKAtomicNumber64::operator += (Value value)
{
	Add(value);
	return *this;
}

DKAtomicNumber64::operator Value () const
{
	return (Value)atomic;
}
