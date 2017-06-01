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
	, entryPoint("main")
{
}

DKShader::DKShader(const void* data, size_t size, StageType st, const DKString& entry)
	: stage(st)
	, entryPoint(entry)
{
	if (data && size > 0)
	{
		codeData = DKOBJECT_NEW DKBuffer(data, size);
	}
}

DKShader::DKShader(DKData* data, StageType st, const DKString& entry)
	: stage(st)
	, entryPoint(entry)
	, codeData(data)
{
}

DKShader::~DKShader(void)
{
}
