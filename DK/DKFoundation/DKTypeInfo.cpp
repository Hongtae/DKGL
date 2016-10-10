//
//  File: DKTypeInfo.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKTypes.h"
#include "DKTypeInfo.h"
#include "DKString.h"

namespace DKGL
{
	namespace Private
	{
		class InvalidType {};
	}
}

using namespace DKGL;

DKTypeInfo::DKTypeInfo(void)
	: info(&typeid(Private::InvalidType))
{
}

DKTypeInfo::DKTypeInfo(const DKTypeInfo& ti)
	: info(ti.info)
{
}

DKTypeInfo::DKTypeInfo(const std::type_info& ti)
	: info(&ti)
{
}

DKTypeInfo::~DKTypeInfo(void)
{
}

bool DKTypeInfo::IsValid(void) const
{
	return *info != typeid(Private::InvalidType);
}

bool DKTypeInfo::Before(const DKTypeInfo& rhs) const
{
	return info->before(*rhs.info) != 0;
}

DKTypeInfo::operator const std::type_info& (void) const
{
	return *info;
}

DKString DKTypeInfo::Name(void) const
{
	return DKString(info->name());
}

DKTypeInfo& DKTypeInfo::operator = (const DKTypeInfo& ti)
{
	info = ti.info;
	return *this;
}

bool DKTypeInfo::operator == (const DKTypeInfo& rhs) const
{
	return *info == *rhs.info;
}

bool DKTypeInfo::operator != (const DKTypeInfo& rhs) const
{
	return *info != *rhs.info;
}

bool DKTypeInfo::operator < (const DKTypeInfo& rhs) const
{
	return Before(rhs);
}

bool DKTypeInfo::operator > (const DKTypeInfo& rhs) const
{
	return rhs < *this;
}

bool DKTypeInfo::operator <= (const DKTypeInfo& rhs) const
{
	return !(*this > rhs);
}

bool DKTypeInfo::operator >= (const DKTypeInfo& rhs) const
{
	return !(*this < rhs); 
}
