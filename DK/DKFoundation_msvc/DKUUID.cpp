//
//  File: DKUUID.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKUUID.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"
#include "DKEndianness.h"

#ifdef _WIN32
#include <Windows.h>
#include <Rpc.h>
#pragma comment (lib, "rpcrt4")
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <uuid/uuid.h>
#include <string.h>
#endif
#if defined(__linux__)
#include <uuid/uuid.h>
#endif

using namespace DKFoundation;

DKUUID::DKUUID(void)
{
	int* v = reinterpret_cast<int*>(data);
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
	v[3] = 0;
}

DKUUID::DKUUID(const DKUUID& uuid)
{
	int* v1 = reinterpret_cast<int*>(data);
	const int* v2 = reinterpret_cast<const int*>(uuid.data);
	v1[0] = v2[0];
	v1[1] = v2[1];
	v1[2] = v2[2];
	v1[3] = v2[3];
}

void DKUUID::SetZero(void)
{
	int* v = reinterpret_cast<int*>(data);
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
	v[3] = 0;
}

DKUUID::DKUUID(const DKString& str)
{
	memset(this->data, 0, sizeof(this->data));
	size_t inputLength = str.Length();
	if (inputLength >= 16)
	{
		unsigned char buff[16];
		auto byteFromHex = [](wchar_t c) -> unsigned char
		{
			if (( c >= L'0') && (c <= L'9'))
				return static_cast<unsigned char>(c - L'0');
			else if ((c >= L'a') && (c <= L'f'))
				return static_cast<unsigned char>(c - (L'a' - 10));
			else if ((c >= L'A') && (c <= L'F'))
				return static_cast<unsigned char>(c - (L'A' - 10));
			return 0;
		};
		auto isHexChar = [](wchar_t c)->bool
		{
			return ((
					 ((c >= L'0') && (c <= L'9')) ||
					 ((c >= L'a') && (c <= L'f')) ||
					 ((c >= L'A') && (c <= L'F'))) ? true : false);
		};

		const wchar_t* inputStr = str;

		// 스트링 파싱
		size_t index = 0;
		for (size_t i = 0; i < inputLength && index < 16;)
		{
			if (isHexChar(inputStr[i]) && isHexChar(inputStr[i+1]))
			{
				char a = byteFromHex(inputStr[i]);
				char b = byteFromHex(inputStr[i+1]);
				buff[index] = (a << 4) | b;
				index++;
				i+=2;
			}
			else
			{
				i++;
			}
		}

		if (index == 16)
			memcpy(this->data, buff, 16);		
	}
}

DKUUID DKUUID::Create(void)
{
	DKUUID uuid;
	static DKSpinLock lock;
	DKCriticalSection<DKSpinLock> guard(lock);

#ifdef _WIN32
	UUID u;
	RPC_STATUS st = UuidCreate(&u);
	if (RPC_S_OK != st && RPC_S_UUID_LOCAL_ONLY != st)
	{
	}
	else
	{
		u.Data1 = DKSystemToBigEndian(u.Data1);
		u.Data2 = DKSystemToBigEndian(u.Data2);
		u.Data3 = DKSystemToBigEndian(u.Data3);

		memcpy(uuid.data, &u, sizeof(uuid.data));
	}
#else
	uuid_t u;
	uuid_generate_random(u);
	memcpy(uuid.data, u, sizeof(uuid.data));
#endif
	return uuid;
}

DKString DKUUID::String(void) const
{
	char buff[40];

	auto hexChars = [](char in, char* out)
	{
		int digits = 2;
		while (--digits >= 0)
		{
			int shift = digits << 2;
			int d = 0x0F & (in >> shift);
			if (d <= 9)
				*out++ = '0' + d;
			else
				*out++ = 'A' + (d - 10);
		}
	};

	int i = 0;
	int j = 0;
	for (; i < 4; ++i, j+=2)
		hexChars(this->data[i], &buff[j]);	// first-8
	buff[j++] = '-';
	for (; i < 6; ++i, j+=2)
		hexChars(this->data[i], &buff[j]);	// second-4
	buff[j++] = '-';
	for (; i < 8; ++i, j+=2)
		hexChars(this->data[i], &buff[j]);	// third-4
	buff[j++] = '-';
	for (; i < 10; ++i, j+=2)
		hexChars(this->data[i], &buff[j]);	// fourth-4
	buff[j++] = '-';
	for (; i < 16; ++i, j+=2)
		hexChars(this->data[i], &buff[j]);	// fifth-12
	buff[j++] = 0;
	
	return DKString(buff);
}

int DKUUID::Compare(const DKUUID& rhs) const
{
	int cmp = 0;
	for (int i = 0; i < 16 && cmp == 0; ++i)
	{
		cmp = data[i] - rhs.data[i];
	}
	return cmp;
}

DKUUID& DKUUID::operator = (const DKUUID& uuid)
{
	int* v1 = reinterpret_cast<int*>(data);
	const int* v2 = reinterpret_cast<const int*>(uuid.data);
	v1[0] = v2[0];
	v1[1] = v2[1];
	v1[2] = v2[2];
	v1[3] = v2[3];
	return *this;
}

bool DKUUID::IsValid(void) const
{
	return IsZero() == false;
}

bool DKUUID::IsZero(void) const
{
	const int* iv = reinterpret_cast<const int*>(this->data);
	return iv[0] == 0 && iv[1] == 0 && iv[2] == 0 && iv[3] == 0;
}
