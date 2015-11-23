//
//  File: DKSceneState.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKSceneState.h"

using namespace DKFoundation;
using namespace DKFramework;


void DKSceneState::Clear(void)
{
	this->directionalLightColors.Clear();
	this->directionalLightDirections.Clear();
	this->pointLightColors.Clear();
	this->pointLightPositions.Clear();
	this->pointLightAttenuations.Clear();
	this->ClearModel();
}

void DKSceneState::ClearModel(void)
{
	this->modelMatrix.Identity();
	this->modelMatrixInverse.Identity();
	this->modelViewMatrix.Identity();
	this->modelViewMatrixInverse.Identity();
	this->modelViewProjectionMatrix.Identity();
	this->modelViewProjectionMatrixInverse.Identity();

	this->linearTransformMatrixArray.Clear();
	this->affineTransformMatrixArray.Clear();

	this->materialProperties = NULL;
	this->materialSamplers = NULL;
}

DKSceneState::IntArray DKSceneState::GetIntProperty(const DKShaderConstant& sc, int programIndex)
{
	if (this->userMaterialPropertyCallback)
	{
		auto val = this->userMaterialPropertyCallback->GetIntProperty(sc, programIndex);
		if (val.Count() > 0)
			return val;
	}
	if (this->materialProperties)
	{
		auto p = this->materialProperties->Find(sc.name);
		if (p && p->value.integers.Count() > 0)
			return IntArray((int*)(const int*)p->value.integers, p->value.integers.Count());
	}
	return IntArray();
}

DKSceneState::FloatArray DKSceneState::GetFloatProperty(const DKShaderConstant& sc, int programIndex)
{
	switch (sc.id)
	{
	case DKShaderConstant::UniformModelMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->modelMatrix.val, 16);
		break;
	case DKShaderConstant::UniformModelMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
		return FloatArray(this->modelMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformViewMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
		return FloatArray(this->viewMatrix.val, 16);
		break;
	case DKShaderConstant::UniformViewMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
		return FloatArray(this->viewMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformProjectionMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
		return FloatArray(this->projectionMatrix.val, 16);
		break;
	case DKShaderConstant::UniformProjectionMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
		return FloatArray(this->projectionMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformViewProjectionMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->viewProjectionMatrix.val, 16);
		break;
	case DKShaderConstant::UniformViewProjectionMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->viewProjectionMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformModelViewMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->modelViewMatrix.val, 16);
		break;
	case DKShaderConstant::UniformModelViewMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->modelViewMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformModelViewProjectionMatrix:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->modelViewProjectionMatrix.val, 16);
		break;
	case DKShaderConstant::UniformModelViewProjectionMatrixInverse:
		if (sc.type == DKShaderConstant::TypeFloat4x4)
			return FloatArray(this->modelViewProjectionMatrixInverse.val, 16);
		break;
	case DKShaderConstant::UniformLinearTransformArray:
		if (sc.type == DKShaderConstant::TypeFloat3x3 && this->linearTransformMatrixArray.Count() > 0)
			return FloatArray(this->linearTransformMatrixArray.Value(0).val, this->linearTransformMatrixArray.Count() * 9);
		break;
	case DKShaderConstant::UniformAffineTransformArray:
		if (sc.type == DKShaderConstant::TypeFloat4x4 && this->affineTransformMatrixArray.Count() > 0)
			return FloatArray(this->affineTransformMatrixArray.Value(0).val, this->affineTransformMatrixArray.Count() * 16);
		break;
	case DKShaderConstant::UniformPositionArray:
		if (sc.type == DKShaderConstant::TypeFloat3 && this->positionArray.Count() > 0)
			return FloatArray(this->positionArray.Value(0).val, this->positionArray.Count() * 3);
		break;
	case DKShaderConstant::UniformAmbientColor:
		if (sc.type == DKShaderConstant::TypeFloat3 || sc.type == DKShaderConstant::TypeFloat4)
			return FloatArray(this->ambientColor.val, 4);
		break;
	case DKShaderConstant::UniformCameraPosition:
		if (sc.type == DKShaderConstant::TypeFloat3)
			return FloatArray(this->cameraPosition.val, 3);
		break;
	default:
		break;
	}

	if (this->userMaterialPropertyCallback)
	{
		FloatArray val = this->userMaterialPropertyCallback->GetFloatProperty(sc, programIndex);
		if (val.Count() > 0)
			return val;
	}
	if (this->materialProperties)
	{
		auto p = this->materialProperties->Find(sc.name);
		if (p && p->value.floatings.Count() > 0)
			return FloatArray((float*)(const float*)p->value.floatings, p->value.floatings.Count());
	}
	return FloatArray();
}

const DKSceneState::Sampler* DKSceneState::GetSamplerProperty(const DKShaderConstant& sc, int programIndex)
{
	if (this->userMaterialPropertyCallback)
	{
		auto p = this->userMaterialPropertyCallback->GetSamplerProperty(sc, programIndex);
		if (p && p->textures.Count() > 0)
			return p;
	}
	if (this->materialSamplers)
	{
		auto p = this->materialSamplers->Find(sc.name);
		if (p && p->value.textures.Count() > 0)
			return &p->value;
	}
	return NULL;
}
