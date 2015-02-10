//
//  File: DKUUID.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

////////////////////////////////////////////////////////////////////////////////
// DKUUID
// UUID generate and represent as string.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKUUID
	{
	public:
		DKUUID(void);
		DKUUID(const DKUUID&);
		DKUUID(const DKString&);
		static DKUUID Create(void);
		
		DKString String(void) const;

		DKUUID& operator = (const DKUUID&);

		bool IsValid(void) const;
		bool IsZero(void) const;
		int Compare(const DKUUID&) const;

		bool operator == (const DKUUID& rhs) const	{return Compare(rhs) == 0;}
		bool operator != (const DKUUID& rhs) const	{return Compare(rhs) != 0;}
		bool operator > (const DKUUID& rhs) const	{return Compare(rhs) > 0;}
		bool operator >= (const DKUUID& rhs) const	{return Compare(rhs) >= 0;}
		bool operator < (const DKUUID& rhs) const	{return Compare(rhs) < 0;}
		bool operator <= (const DKUUID& rhs) const	{return Compare(rhs) <= 0;}

		void SetZero(void);

	private:
		unsigned char data[16];
	};
}
