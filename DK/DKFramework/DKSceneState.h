//
//  File: DKSceneState.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKLight.h"
#include "DKColor.h"
#include "DKMaterial.h"
#include "DKCamera.h"

////////////////////////////////////////////////////////////////////////////////
// DKSceneState
// scene state used when drawing one scene. (DKScene)
// scene state is global scene state for scene.
// individual object's state can be overriden by object itself.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKSceneState : public DKMaterial::PropertyCallback
	{
	public:
		template <typename T> using StringKeyMap = DKFoundation::DKMap<DKFoundation::DKString, T>;

		typedef DKFoundation::DKArray<DKVector2> Vector2Array;
		typedef DKFoundation::DKArray<DKVector3> Vector3Array;
		typedef DKFoundation::DKArray<DKVector4> Vector4Array;
		typedef DKFoundation::DKArray<DKMatrix2> Matrix2Array;
		typedef DKFoundation::DKArray<DKMatrix3> Matrix3Array;
		typedef DKFoundation::DKArray<DKMatrix4> Matrix4Array;
		typedef StringKeyMap<DKMaterial::PropertyArray> PropertyMap;
		typedef StringKeyMap<DKMaterial::Sampler> SamplerMap;

		unsigned int sceneIndex = 0;

		// camera matrices
		DKMatrix4	viewMatrix;
		DKMatrix4	viewMatrixInverse;
		DKMatrix4	projectionMatrix;
		DKMatrix4	projectionMatrixInverse;
		DKMatrix4	viewProjectionMatrix;
		DKMatrix4	viewProjectionMatrixInverse;

		// camera info
		DKVector3	cameraPosition;
		DKColor		ambientColor;

		// lighting info
		Vector3Array directionalLightColors;
		Vector3Array directionalLightDirections;
		Vector3Array pointLightColors;
		Vector3Array pointLightPositions;
		Vector3Array pointLightAttenuations;		// (const, linear, quadratic)

		// model(mesh) transform matrices
		DKMatrix4 modelMatrix;
		DKMatrix4 modelMatrixInverse;
		DKMatrix4 modelViewMatrix;
		DKMatrix4 modelViewMatrixInverse;
		DKMatrix4 modelViewProjectionMatrix;
		DKMatrix4 modelViewProjectionMatrixInverse;
		Matrix3Array linearTransformMatrixArray;
		Matrix4Array affineTransformMatrixArray;
		Vector3Array positionArray;

		// model(mesh) shading properties
		const PropertyMap* materialProperties = NULL;
		const SamplerMap* materialSamplers = NULL;

		// user material callback
		DKFoundation::DKObject<DKMaterial::PropertyCallback> userMaterialPropertyCallback;

		IntArray GetIntProperty(const DKShaderConstant& sc, int programIndex) override;
		FloatArray GetFloatProperty(const DKShaderConstant& sc, int programIndex) override;
		const Sampler* GetSamplerProperty(const DKShaderConstant& sc, int programIndex) override;

		void Clear(void);
		void ClearModel(void);
	};
}
