//
//  File: DKEndianness.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#include "DKEndianness.h"


namespace DKFoundation
{
	bool DKByteOrderCheck(void)
	{
		struct DKByteOrderRuntime
		{
			enum RTByteOrder
			{
				RTByteOrderUnknown,
				RTByteOrderBigEndian,
				RTByteOrderLittleEndian,
			};
			static inline RTByteOrder RuntimeEndianness(void)
			{
				union
				{
					unsigned int s;
					char n[4];
				} val = {(unsigned int)'RTBO'};
				if (val.n[0] == 'R' && val.n[1] == 'T' && val.n[2] == 'B' && val.n[3] == 'O')
					return RTByteOrderBigEndian;
				if (val.n[0] == 'O' && val.n[1] == 'B' && val.n[2] == 'T' && val.n[3] == 'R')
					return RTByteOrderLittleEndian;
				return RTByteOrderUnknown;
			}
			static inline bool Verify(void)
			{
#ifdef __BIG_ENDIAN__
				return RuntimeEndianness() == RTByteOrderBigEndian;
#endif
#ifdef __LITTLE_ENDIAN__
				return RuntimeEndianness() == RTByteOrderLittleEndian;
#endif
				return false;
			}
		};
		return DKByteOrderRuntime::Verify();
	}
}
