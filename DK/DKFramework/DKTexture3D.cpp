//
//  File: DKTexture3D.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKTexture3D.h"
#include "DKOpenGLContext.h"


using namespace DKGL;
using namespace DKGL;


DKTexture3D::DKTexture3D(void)
	: DKTexture(Target3D)
{
}

DKTexture3D::~DKTexture3D(void)
{

}

DKObject<DKSerializer> DKTexture3D::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKTexture3D* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKTexture3D");
			this->Bind(L"super", target->DKTexture::Serializer(), NULL);

			return this;
		}
	private:
		DKObject<DKTexture3D> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
