//
//  File: DKShader.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKShader.h"

using namespace DKFramework;

DKShader::DKShader(void)
	: stage(StageType::Vertex)
{
}

DKShader::DKShader(const void* data, size_t size, StageType st)
	: stage(st)
{
	if (data && size > 0)
	{
		codeData = DKOBJECT_NEW DKBuffer(data, size);
	}
}

DKShader::DKShader(DKData* data, StageType st)
	: stage(st)
	, codeData(data)
{
}

DKShader::~DKShader(void)
{
}
