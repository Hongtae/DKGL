//
//  File: DKUuid.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

////////////////////////////////////////////////////////////////////////////////
// DKUuid
// UUID generate and represent as string.
////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 4)
namespace DKFoundation
{
	class DKGL_API DKUuid
	{
	public:
		DKUuid(void);
		DKUuid(const DKUuid&);
		DKUuid(const DKString&);
		static DKUuid Create(void);
		
		DKString String(void) const;

		DKUuid& operator = (const DKUuid&);

		bool IsValid(void) const;
		bool IsZero(void) const;
		int Compare(const DKUuid&) const;

		bool operator == (const DKUuid& rhs) const	{return Compare(rhs) == 0;}
		bool operator != (const DKUuid& rhs) const	{return Compare(rhs) != 0;}
		bool operator > (const DKUuid& rhs) const	{return Compare(rhs) > 0;}
		bool operator >= (const DKUuid& rhs) const	{return Compare(rhs) >= 0;}
		bool operator < (const DKUuid& rhs) const	{return Compare(rhs) < 0;}
		bool operator <= (const DKUuid& rhs) const	{return Compare(rhs) <= 0;}

		void SetZero(void);

	private:
		unsigned char data[16];
	};
}
#pragma pack(pop)
