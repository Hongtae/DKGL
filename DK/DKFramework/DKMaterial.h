//
//  File: DKMaterial.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKColor.h"
#include "DKShader.h"
#include "DKShaderProgram.h"
#include "DKTexture.h"
#include "DKShaderConstant.h"
#include "DKVertexStream.h"
#include "DKBlendState.h"
#include "DKRenderState.h"
#include "DKTextureSampler.h"

////////////////////////////////////////////////////////////////////////////////
// DKMaterial
// Material for DKMesh.
// One material object can have several shader programs.
// (defined in RenderingProperty), It makes one material to be used in multiple
// rendering passes.
// Material object's rendering properties can be accessed by index.
// Material object can define additional properties which used if property has
// been omitted from mesh object.
// One material object can be shared by multiple mesh objects.
//
// This class is low-level shader and shader programs manager, and does not
// provides any of shader composing feature. You need to write your own shader
// code. (codes can be serialized)
//
// Note:
//   Matrix will be transposed (as Column-major order) when transfer to shader.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKMaterial : public DKResource
	{
	public:

		// PropertyArray
		// Property value object that can be used by ShadingProperty.
		struct PropertyArray
		{
			DKArray<int> integers;
			DKArray<float> floatings;

			PropertyArray(void) {}
			PropertyArray(int value, size_t count = 1) : integers(value, count) {}
			PropertyArray(const int* value, size_t count) : integers(value, count) {}
			PropertyArray(float value, size_t count = 1) : floatings(value, count) {}
			PropertyArray(const float* value, size_t count) : floatings(value, count) {}
		};

		typedef DKArray<DKObject<DKTexture>> TextureArray;
		struct Sampler
		{
			TextureArray								textures;
			DKObject<DKTextureSampler>	sampler;
		};

		// SamplerProperty
		// Textures and sampler object.
		struct SamplerProperty
		{
			DKShaderConstant::Uniform					id;				// sampler
			DKShaderConstant::Type						type;
			TextureArray								textures;
			DKObject<DKTextureSampler>	sampler;
		};

		// ShadingProperty
		// Used when shader uniform matching, default value can be defined.
		// uniform values can be acquired from mesh object when rendering.
		// if value is missing, default value used.
		// You can define your own uniform with UniformUserDefine type id.
		// Predefined Uniform IDs are used in DKSceneState object.
		struct ShadingProperty
		{
			DKShaderConstant::Uniform		id;
			DKShaderConstant::Type			type;
			PropertyArray					value; // default value, can be used if id is 'UniformUserDefine'
		};
		// StreamProperty
		// Object for shader attribute stream matching.
		// (see DKVertexStream.h)
		struct StreamProperty
		{
			DKVertexStream::Stream		id;
			DKVertexStream::Type		type;
			size_t						components;
		};
		// ShaderSource
		// a shader source, can have multiple sources combines to one program.
		// sources can be shared by multiple programs (in RenderingProperty)
		struct ShaderSource
		{
			DKString				name;
			DKString				source;
			DKShader::Type						type;
			DKObject<DKShader>	shader;
		};
		// RenderingProperty
		// Used when rendering. This class has shader program which used by
		// scene drawing. This class can have shader sources also.
		// sources are subordinate to program.
		struct RenderingProperty
		{
			enum DepthFunc : unsigned char
			{
				DepthFuncNever = 0,
				DepthFuncAlways,
				DepthFuncLess,
				DepthFuncLessEqual,
				DepthFuncEqual,
				DepthFuncGreater,
				DepthFuncGreaterEqual,
				DepthFuncNotEqual,
			};

			DKString						name;
			DepthFunc									depthFunc;
			bool										depthWrite;
			DKBlendState								blendState;
			DKArray<ShaderSource>			shaders;
			DKObject<DKShaderProgram>		program;
		};

		class PropertyCallback
		{
		public:
			typedef DKStaticArray<int> IntArray;
			typedef DKStaticArray<float> FloatArray;
			typedef DKMaterial::Sampler Sampler;

			virtual IntArray GetIntProperty(const DKShaderConstant&, int) = 0;
			virtual FloatArray GetFloatProperty(const DKShaderConstant&, int) = 0;
			virtual const Sampler* GetSamplerProperty(const DKShaderConstant&, int) = 0;
			virtual ~PropertyCallback(void) {}
		};

		DKMaterial(void);
		~DKMaterial(void);

		int IndexOfRenderPropertyName(const DKString& name) const;
		size_t NumberOfRenderProperties(void) const;

		bool Bind(int programIndex, PropertyCallback* callback, const DKBlendState* blending = NULL) const;

		const DKArray<DKVertexStream>& StreamArray(int state) const;

		struct BuildLog
		{
			DKString errorLog;
			DKString failedShader;
			DKString failedProgram;
		};
		static bool CompileShaderSource(ShaderSource* src, DKString& errorLog);
		bool BuildProgram(int index, BuildLog* log);
		bool Build(BuildLog* log);

		// cleanup shader compiler resources.
		static void ReleaseShaderCompiler(void);

		DKObject<DKSerializer> Serializer(void);

		typedef DKMap<DKString, ShadingProperty>	ShadingPropertyMap;
		typedef DKMap<DKString, SamplerProperty>	SamplerPropertyMap;
		typedef DKMap<DKString, StreamProperty>		StreamPropertyMap;
		typedef DKArray<ShaderSource>								ShaderSourceArray;
		typedef DKArray<RenderingProperty>						RenderingPropertyArray;

		// Default properties of material object.
		// This is fallback values, used if property callbacks did not returns proper value.
		ShadingPropertyMap				shadingProperties;
		SamplerPropertyMap				samplerProperties;
		StreamPropertyMap				streamProperties;
		ShaderSourceArray				shaderProperties;
		RenderingPropertyArray			renderingProperties;

		bool Validate(void) override;
		bool IsValid(void) const;
	};
}
