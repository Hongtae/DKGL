//
//  File: DKUuid.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

#pragma pack(push, 4)
namespace DKFoundation
{
	/// UUID generate and represent as a string.
	class DKGL_API DKUuid
	{
	public:
		DKUuid();
		DKUuid(const DKUuid&);
		DKUuid(const DKString&);
		static DKUuid Create();
		
		DKString String() const;

		DKUuid& operator = (const DKUuid&);

		bool IsValid() const;
		bool IsZero() const;
		int Compare(const DKUuid&) const;

		bool operator == (const DKUuid& rhs) const	{return Compare(rhs) == 0;}
		bool operator != (const DKUuid& rhs) const	{return Compare(rhs) != 0;}
		bool operator > (const DKUuid& rhs) const	{return Compare(rhs) > 0;}
		bool operator >= (const DKUuid& rhs) const	{return Compare(rhs) >= 0;}
		bool operator < (const DKUuid& rhs) const	{return Compare(rhs) < 0;}
		bool operator <= (const DKUuid& rhs) const	{return Compare(rhs) <= 0;}

		void SetZero();

	private:
		unsigned char data[16];
	};
}
#pragma pack(pop)
