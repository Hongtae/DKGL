//
//  File: DKSoftBody.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletPhysics.h"
#include "DKSoftBody.h"

using namespace DKFramework;


DKSoftBody::DKSoftBody()
: DKCollisionObject(ObjectType::SoftBody, new btSoftBody(nullptr))
{
	DKERROR_THROW("Not Implemented");
}

DKSoftBody::~DKSoftBody()
{
}

DKObject<DKModel> DKSoftBody::Clone(UUIDObjectMap&) const
{
	DKERROR_THROW("Not Implemented");
	return NULL;
}

DKObject<DKSerializer> DKSoftBody::Serializer()
{
	DKERROR_THROW("Not Implemented");
	return NULL;
}
