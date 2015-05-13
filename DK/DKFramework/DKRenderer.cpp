//
//  File: DKRenderer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "Private/BulletUtils.h"

#include "DKMath.h"
#include "DKRenderer.h"
#include "DKOpenGLContext.h"
#include "DKRenderState.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"
#include "DKLinearTransform2.h"
#include "DKLinearTransform3.h"
#include "DKAffineTransform2.h"
#include "DKAffineTransform3.h"
#include "DKColor.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKLine.h"
#include "DKSphere.h"
#include "DKAABox.h"
#include "DKStaticMesh.h"

#include "DKTexture.h"
#include "DKTextureSampler.h"
#include "DKFont.h"
#include "DKMesh.h"
#include "DKScene.h"
#include "DKSceneState.h"
#include "DKRenderState.h"

using namespace DKFoundation;

namespace DKFramework
{
	static_assert(sizeof(DKPoint) == sizeof(float) * 2, "size mismatch");
	static_assert(sizeof(DKSize) == sizeof(float) * 2, "size mismatch");
	static_assert(sizeof(DKRect) == sizeof(float) * 4, "size mismatch");

	static_assert(sizeof(DKColor) == sizeof(float) * 4, "size mismatch");
	static_assert(sizeof(DKQuaternion) == sizeof(float) * 4, "size mismatch");

	static_assert(sizeof(DKVector2) == sizeof(float) * 2, "size mismatch");
	static_assert(sizeof(DKVector3) == sizeof(float) * 3, "size mismatch");
	static_assert(sizeof(DKVector4) == sizeof(float) * 4, "size mismatch");

	static_assert(sizeof(DKMatrix2) == sizeof(float) * 4, "size mismatch");
	static_assert(sizeof(DKMatrix3) == sizeof(float) * 9, "size mismatch");
	static_assert(sizeof(DKMatrix4) == sizeof(float) * 16, "size mismatch");

	static_assert(sizeof(DKLinearTransform2) == sizeof(float) * 4, "size mismatch");
	static_assert(sizeof(DKLinearTransform3) == sizeof(float) * 9, "size mismatch");
	static_assert(sizeof(DKAffineTransform2) == sizeof(float) * 6, "size mismatch");
	static_assert(sizeof(DKAffineTransform3) == sizeof(float) * 12, "size mismatch");

	static_assert(sizeof(DKLine) == sizeof(float) * 6, "size mismatch");
	static_assert(sizeof(DKSphere) == sizeof(float) * 4, "size mismatch");
	static_assert(sizeof(DKAABox) == sizeof(float) * 6, "size mismatch");

	namespace Private
	{

#define DKLIB_GLSL_PRECISION "#ifndef GL_ES\n#define highp\n#define mediump\n#define lowp\n#endif\n"

		static DKMaterial::ShaderSource vertexShader2T =
		{
			L"vertexShader2T",
			DKLIB_GLSL_PRECISION
			"attribute mediump vec2 position;\n"
			"attribute mediump vec2 texCoord;\n"
			"varying   mediump vec2 textureCoord;\n"
			"varying   mediump vec2 currentPosition;\n"
			"void main(void) {\n"
			"    currentPosition = position;\n"
			"    textureCoord = texCoord;\n"
			"    gl_Position = vec4(position, 0, 1);\n"
			"}\n",
			DKShader::TypeVertexShader,
			0
		};
		static DKMaterial::ShaderSource vertexShader3TC =
		{
			L"vertexShader3TC",
			DKLIB_GLSL_PRECISION
			"uniform   highp   mat4 transform;\n"
			"attribute highp   vec3 position;\n"
			"attribute mediump vec2 texCoord;\n"
			"attribute lowp    vec4 vertColor;\n"
			"varying   mediump vec2 textureCoord;\n"
			"varying   lowp    vec4 color;\n"
			"void main(void) {\n"
			"    textureCoord = texCoord;\n"
			"    color = vertColor;\n"
			"    gl_Position = transform * vec4(position, 1);\n"
			"}\n",
			DKShader::TypeVertexShader,
			0
		};
		static DKMaterial::ShaderSource vertexShader3C =
		{
			L"vertexShader3C",
			DKLIB_GLSL_PRECISION
			"uniform   highp mat4 transform;\n"
			"attribute highp vec3 position;\n"
			"attribute lowp  vec4 vcolor;\n"
			"varying   lowp  vec4 color;\n"
			"void main(void) {\n"
			"    color = vcolor;\n"
			"    gl_Position = transform * vec4(position, 1);\n"
			"}\n",
			DKShader::TypeVertexShader,
			0
		};
		static DKMaterial::ShaderSource uniformColorFragmentShader =
		{
			L"uniformColorFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform lowp vec4 color;\n"
			"void main(void) {\n"
			"    gl_FragColor = color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource varyingColorFragmentShader =
		{
			L"varyingColorFragmentShader",
			DKLIB_GLSL_PRECISION
			"varying lowp vec4 color;\n"
			"void main(void) {\n"
			"    gl_FragColor = color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource uniformColorTextureFragmentShader =
		{
			L"uniformColorTextureFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform sampler2D    tex;\n"
			"uniform lowp    vec4 color;\n"
			"varying mediump vec2 textureCoord;\n"
			"void main(void) {\n"
			"    gl_FragColor = texture2D(tex, textureCoord) * color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource varyingColorTextureFragmentShader =
		{
			L"varyingColorTextureFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform sampler2D    tex;\n"
			"varying mediump vec2 textureCoord;\n"
			"varying lowp    vec4 color;\n"
			"void main(void) {\n"
			"    gl_FragColor = texture2D(tex, textureCoord) * color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource solidEllipseFragmentShader =
		{
			L"solidEllipseFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform mediump vec2 radiusSq;\n" // vec2(A^2,B^2) value from formula X^2 / A^2 + Y^2 / B^2 = 1
			"uniform mediump vec2 center;\n"   // center of ellipse
			"uniform lowp    vec4 color;\n"
			"varying mediump vec2 currentPosition;\n"
			"void main(void) {\n"
			"    mediump vec2 vl = currentPosition - center;\n"
			"    mediump float form = (vl.x * vl.x)/radiusSq.x + (vl.y * vl.y)/radiusSq.y;\n"
			"    if (form > 1.0) discard;"
			"    gl_FragColor = color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource textureEllipseFragmentShader =
		{
			L"textureEllipseFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform sampler2D    tex;\n"
			"uniform mediump vec2 radiusSq;\n" // vec2(A^2,B^2) value from formula X^2 / A^2 + Y^2 / B^2 = 1
			"uniform mediump vec2 center;\n"   // center of ellipse
			"uniform lowp    vec4 color;\n"
			"varying mediump vec2 currentPosition;\n"
			"varying mediump vec2 textureCoord;\n"
			"void main(void) {\n"
			"    mediump vec2 vl = currentPosition - center;\n"
			"    mediump float form = (vl.x * vl.x)/radiusSq.x + (vl.y * vl.y)/radiusSq.y;\n"
			"    if (form > 1.0) discard;"
			"    gl_FragColor = texture2D(tex, textureCoord) * color;\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};
		static DKMaterial::ShaderSource alphaTextureFragmentShader =
		{
			L"alphaTextureFragmentShader",
			DKLIB_GLSL_PRECISION
			"uniform sampler2D    tex;\n"
			"uniform lowp    vec4 color;\n"
			"varying mediump vec2 textureCoord;\n"
			"void main(void) {\n"
			"    gl_FragColor = vec4(color.rgb, texture2D(tex, textureCoord).a * color.a);\n"
			"}\n",
			DKShader::TypeFragmentShader,
			0
		};

		enum RendererProgram2D
		{
			RP2Colored = 0,
			RP2Textured,
			RP2SolidEllipse,
			RP2TexturedEllipse,
			RP2AlphaTextured,
		};
		enum RenderProgram3D
		{
			RP3Colored = 0,
			RP3Textured,
		};

		static DKObject<DKStaticMesh> Build2DMesh(DKVertexBuffer::MemoryLocation loc, DKVertexBuffer::BufferUsage usage, DKTexture* fallbackTexture)
		{
			auto GetRenderProperty2D = [](const DKFoundation::DKString& name,
				std::initializer_list<const DKMaterial::ShaderSource*> shaders) -> DKMaterial::RenderingProperty
			{
				DKMaterial::RenderingProperty rp =
				{
					name, DKMaterial::RenderingProperty::DepthFuncAlways, false,
					DKBlendState::defaultAlpha, DKArray<DKMaterial::ShaderSource>(), NULL
				};
				for (const DKMaterial::ShaderSource* s : shaders)
				{
					if (s)	rp.shaders.Add(*s);
				}
				return rp;
			};

			// build material
			DKObject<DKMaterial>	material = DKObject<DKMaterial>::New();
			// in vec2 defaultPosition
			DKMaterial::StreamProperty position = { DKVertexStream::StreamPosition, DKVertexStream::TypeFloat2, 1 };
			DKMaterial::StreamProperty texCoord = { DKVertexStream::StreamTexCoord, DKVertexStream::TypeFloat2, 1 };
			// uniform positions, textureCoords, sphereRadius, textureRadius, positionRects, textureRects, screen, color;
			DKMaterial::ShadingProperty dummy = { DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeFloat4, DKMaterial::PropertyArray(0.0f, 4) };
			// sampler2D tex
			DKMaterial::SamplerProperty	tex = { DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeSampler2D,
				fallbackTexture ? DKMaterial::TextureArray(fallbackTexture, 1) : DKMaterial::TextureArray(), NULL };

			material->renderingProperties.Add(GetRenderProperty2D(L"solidColor", { &vertexShader2T, 0, &uniformColorFragmentShader }));
			material->renderingProperties.Add(GetRenderProperty2D(L"textureColor", { &vertexShader2T, 0, &uniformColorTextureFragmentShader }));
			material->renderingProperties.Add(GetRenderProperty2D(L"solidEllipse", { &vertexShader2T, 0, &solidEllipseFragmentShader }));
			material->renderingProperties.Add(GetRenderProperty2D(L"textureEllipse", { &vertexShader2T, 0, &textureEllipseFragmentShader }));
			material->renderingProperties.Add(GetRenderProperty2D(L"alphaTexture", { &vertexShader2T, 0, &alphaTextureFragmentShader }));

			material->streamProperties.Insert(L"position", position);
			material->streamProperties.Insert(L"texCoord", texCoord);

			material->shadingProperties.Insert(L"position", dummy);
			material->shadingProperties.Insert(L"textCoord", dummy);
			material->shadingProperties.Insert(L"radiusSq", dummy);
			material->shadingProperties.Insert(L"center", dummy);
			material->shadingProperties.Insert(L"color", dummy);

			material->samplerProperties.Insert(L"tex", tex);

			DKMaterial::BuildLog log;
			if (!material->Build(&log))
			{
				DKLog("Building screen utils error: %ls\n", (const wchar_t*)log.errorLog);
				DKLog("While trying to build shader: %ls, in program: %ls.\n", (const wchar_t*)log.failedShader, (const wchar_t*)log.failedProgram);
				return NULL;
			}
			// cleanup source (for memory)
			material->shaderProperties.Clear();
			for (DKMaterial::RenderingProperty& rp : material->renderingProperties)
				rp.shaders.Clear();

			// build mesh
			const DKVertexBuffer::Decl decl[] = {
					{ DKVertexStream::StreamPosition, L"", DKVertexStream::TypeFloat2, false, 0 },
					{ DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeFloat2, false, DKVertexStream::TypeSize(DKVertexStream::TypeFloat2) }
			};

			DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(decl, 2, NULL, sizeof(float) * 4, 0, loc, usage);

			DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
			mesh->SetDrawFace(DKMesh::DrawFaceBoth);
			mesh->SetDefaultPrimitiveType(DKPrimitive::TypePoints);
			mesh->AddVertexBuffer(vb);
			mesh->SetMaterial(material);

			return mesh;
		}

		static DKObject<DKStaticMesh> Build3DMesh(DKVertexBuffer::MemoryLocation loc, DKVertexBuffer::BufferUsage usage, DKTexture* fallbackTexture)
		{
			auto GetRenderProperty3D = [](const DKFoundation::DKString& name,
				std::initializer_list<const DKMaterial::ShaderSource*> shaders) -> DKMaterial::RenderingProperty
			{
				DKMaterial::RenderingProperty rp =
				{
					name, DKMaterial::RenderingProperty::DepthFuncLessEqual, true,
					DKBlendState::defaultAlpha, DKArray<DKMaterial::ShaderSource>(), NULL
				};
				for (const DKMaterial::ShaderSource* s : shaders)
				{
					if (s)	rp.shaders.Add(*s);
				}
				return rp;
			};

			// build material
			DKObject<DKMaterial>	material = DKObject<DKMaterial>::New();
			// in vec3 defaultPosition
			DKMaterial::StreamProperty position = { DKVertexStream::StreamPosition, DKVertexStream::TypeFloat3, 1 };
			DKMaterial::StreamProperty texCoord = { DKVertexStream::StreamTexCoord, DKVertexStream::TypeFloat2, 1 };
			DKMaterial::StreamProperty vertColor = { DKVertexStream::StreamColor, DKVertexStream::TypeFloat4, 1 };
			// uniform positions, textureCoords, sphereRadius, textureRadius, positionRects, textureRects, screen, color;
			DKMaterial::ShadingProperty dummy = { DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeFloat4, DKMaterial::PropertyArray(0.0f, 4) };
			// uniform transform
			DKMaterial::ShadingProperty transform = { DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeFloat4x4, DKMaterial::PropertyArray(DKMatrix4::identity.val, 16) };
			// sampler2D tex
			DKMaterial::SamplerProperty	tex = { DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeSampler2D,
				fallbackTexture ? DKMaterial::TextureArray(fallbackTexture, 1) : DKMaterial::TextureArray(), NULL };

			material->renderingProperties.Add(GetRenderProperty3D(L"vertexColor", { &vertexShader3TC, 0, &varyingColorFragmentShader }));
			material->renderingProperties.Add(GetRenderProperty3D(L"textureColor", { &vertexShader3TC, 0, &varyingColorTextureFragmentShader }));

			material->streamProperties.Insert(L"position", position);
			material->streamProperties.Insert(L"texCoord", texCoord);
			material->streamProperties.Insert(L"vertColor", vertColor);

			material->shadingProperties.Insert(L"transform", transform);
			material->shadingProperties.Insert(L"position", dummy);
			material->shadingProperties.Insert(L"textCoord", dummy);

			material->samplerProperties.Insert(L"tex", tex);

			DKMaterial::BuildLog log;
			if (!material->Build(&log))
			{
				DKLog("Building screen utils error: %ls\n", (const wchar_t*)log.errorLog);
				DKLog("While trying to build shader: %ls, in program: %ls.\n", (const wchar_t*)log.failedShader, (const wchar_t*)log.failedProgram);
				return NULL;
			}
			// cleanup source (for memory)
			material->shaderProperties.Clear();
			for (DKMaterial::RenderingProperty& rp : material->renderingProperties)
				rp.shaders.Clear();

			// build model
			const DKVertexBuffer::Decl decl[] = {
					{ DKVertexStream::StreamPosition, L"", DKVertexStream::TypeFloat3, false, 0 },
					{ DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeFloat2, false, sizeof(float) * 3 },
					{ DKVertexStream::StreamColor, L"", DKVertexStream::TypeFloat4, false, sizeof(float) * 5 }
			};

			DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(decl, 3, NULL, sizeof(float) * 9, 0, loc, usage);

			DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
			mesh->SetDrawFace(DKMesh::DrawFaceBoth);
			mesh->SetDefaultPrimitiveType(DKPrimitive::TypePoints);
			mesh->AddVertexBuffer(vb);
			mesh->SetMaterial(material);

			return mesh;
		}

		static DKSpinLock						reusableBufferSpinLock;
		static DKArray<DKRenderer::Vertex2D>	reusableVert2DBuffer;   // vertex buffer for 2d
		static DKArray<DKRenderer::Vertex3D>	reusableVert3DBuffer;   // vertex buffer for 3d
	}
}

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

class DKRenderer::RendererContext : public DKFoundation::DKSharedInstance<RendererContext>, public DKUnknown
{
public:
	typedef DKCriticalSection<DKSpinLock> CriticalSection;
	RendererContext(void)
		: screenOrient(DKAffineTransform2(DKLinearTransform2().Scale(2), DKVector2(-1, -1)))
		, vbLoc(DKVertexBuffer::MemoryLocationStream)	// set location to stream, update every frame.
		, vbUsage(DKVertexBuffer::BufferUsageDraw)
		, mesh2D(NULL)
		, mesh3D(NULL)
	{
		unsigned char d[] = { 1, 0, 1 };
		DKObject<DKTexture> dummyTexture = DKTexture2D::Create(1, 1, DKTexture::FormatRGB, DKTexture::TypeUnsignedByte, d).SafeCast<DKTexture>();
		if (dummyTexture == NULL || !dummyTexture->IsValid())
		{
			DKLog("Building dummy texture failed.\n");
		}
		mesh2D = Build2DMesh(vbLoc, vbUsage, dummyTexture);
		mesh3D = Build3DMesh(vbLoc, vbUsage, dummyTexture);
	}
	~RendererContext(void)
	{
		mesh2D = NULL;
		mesh3D = NULL;
	}
	void Update2DMeshStream(DKPrimitive::Type p, const DKRenderer::Vertex2D* vertices, size_t count) const
	{
		mesh2D->VertexBufferAtIndex(0)->UpdateContent(vertices, count, vbLoc, vbUsage);
		mesh2D->SetDefaultPrimitiveType(p);
	}
	void Update3DMeshStream(DKPrimitive::Type p, const DKRenderer::Vertex3D* vertices, size_t count) const
	{
		mesh3D->VertexBufferAtIndex(0)->UpdateContent(vertices, count, vbLoc, vbUsage);
		mesh3D->SetDefaultPrimitiveType(p);
	}
	mutable DKObject<DKStaticMesh>			mesh2D;
	mutable DKObject<DKStaticMesh>			mesh3D;
	DKVertexBuffer::MemoryLocation			vbLoc;
	DKVertexBuffer::BufferUsage				vbUsage;
	const DKAffineTransform2				screenOrient;	// 2D screen-orient transform
	mutable DKSceneState					sceneState;		// scene state of common use
	mutable DKArray<DKRenderer::Vertex2D>	buffer;			// common buffer for 2D rendering
	DKSpinLock								lock;			// lock for DKRenderer
};

const float DKRenderer::minimumScaleFactor = 0.000001f;

DKRenderer::DKRenderer(DKRenderTarget* rt)
	: renderTarget(rt)
	, context(RendererContext::SharedInstance().SafeCast<DKUnknown>())
	, viewport(0, 0, 1, 1)
	, contentBounds(0, 0, 1, 1)
	, contentTM(DKMatrix3::identity)
	, screenTM(DKMatrix3::identity)
	, polygonOffset({ 0.0, 0.0 })
{
	RendererContext* ctxt = GetContext();
	screenTM = ctxt->screenOrient.Matrix3();

	if (renderTarget)
	{
		DKSize size = renderTarget->Resolution();
		this->viewport = DKRect(DKPoint(0, 0), size);
	}
}

DKRenderer::~DKRenderer(void)
{
}

DKRenderer::RendererContext* DKRenderer::GetContext(void) const
{
	DKASSERT_DEBUG(context);
	DKASSERT_DEBUG(context.SafeCast<RendererContext>());
	return const_cast<RendererContext*>(context.StaticCast<RendererContext>());
}

const DKRect& DKRenderer::Viewport(void) const
{
	return viewport;
}

void DKRenderer::SetViewport(const DKRect& rc)
{
	viewport = rc;
	this->UpdateTransform();
}

const DKRect& DKRenderer::ContentBounds(void) const
{
	return contentBounds;
}

void DKRenderer::SetContentBounds(const DKRect& rc)
{
	DKASSERT_DEBUG(rc.size.width > 0.0 && rc.size.height > 0.0);

	this->contentBounds.origin = rc.origin;
	this->contentBounds.size.width = Max(rc.size.width, minimumScaleFactor);
	this->contentBounds.size.height = Max(rc.size.height, minimumScaleFactor);

	this->UpdateTransform();
}

void DKRenderer::SetContentTransform(const DKMatrix3& tm)
{
	this->contentTM = tm;
	this->UpdateTransform();
}

const DKMatrix3& DKRenderer::ContentTransform(void) const
{
	return this->contentTM;
}

void DKRenderer::SetPolygonOffset(float factor, float units)
{
	this->polygonOffset.factor = factor;
	this->polygonOffset.units = units;
}

void DKRenderer::PolygonOffset(float* val) const
{
	if (val)
	{
		val[0] = this->polygonOffset.factor;
		val[1] = this->polygonOffset.units;
	}
}

void DKRenderer::UpdateTransform(void)
{
	const DKPoint& viewportOffset = this->viewport.origin;
	const DKPoint& contentOffset = this->contentBounds.origin;
	const DKSize& contentScale = this->contentBounds.size;

	DKASSERT_DEBUG(contentScale.width > 0.0 && contentScale.height > 0.0);

	RendererContext* ctxt = GetContext();

	DKMatrix3 offsetTM = DKAffineTransform2(-contentOffset.Vector()).Matrix3();
	DKLinearTransform2 s(1.0f / contentScale.width, 1.0f / contentScale.height);
	screenTM = this->contentTM * offsetTM * DKAffineTransform2(s).Multiply(ctxt->screenOrient).Matrix3();
}

DKRenderTarget* DKRenderer::RenderTarget(void)
{
	return renderTarget;
}

const DKRenderTarget* DKRenderer::RenderTarget(void) const
{
	return renderTarget;
}

bool DKRenderer::IsDrawable(void) const
{
	DKASSERT_DEBUG(renderTarget != NULL);
	if (renderTarget->IsValid())
	{
		if (contentBounds.size.width > 0 && contentBounds.size.height > 0 &&
			viewport.size.width > 0 && viewport.size.height > 0)
			return true;
	}
	return false;
}

DKRenderState* DKRenderer::Bind(void) const
{
	if (viewport.size.width > 0 && viewport.size.height > 0)
	{
		if (renderTarget->Bind())
		{
			DKRenderState& state = DKOpenGLContext::RenderState();
			int x, y, w, h;
			x = floor(viewport.origin.x + 0.5f);
			y = floor(viewport.origin.y + 0.5f);
			w = floor(viewport.size.width + 0.5f);
			h = floor(viewport.size.height + 0.5f);
			state.Viewport(x, y, w, h);
			if (this->polygonOffset.factor == 0.0f && this->polygonOffset.units == 0.0f)
			{
				state.Disable(DKRenderState::GLStatePolygonOffsetFill);
			}
			else
			{
				state.Enable(DKRenderState::GLStatePolygonOffsetFill);
				state.PolygonOffset(this->polygonOffset.factor, this->polygonOffset.units);
			}
			return &state;
		}
	}
	return NULL;
}

void DKRenderer::Clear(const DKColor& color) const
{
	DKRenderState* state = this->Bind();
	if (state)
	{
		state->ColorMask(1, 1, 1, 1);    // required to clean color-buffer
		state->DepthMask(true);          // required to clean depth-buffer
		state->ClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void DKRenderer::ClearColorBuffer(const DKColor& color) const
{
	DKRenderState* state = this->Bind();
	if (state)
	{
		state->ColorMask(1, 1, 1, 1);   // required to clean color-buffer
		state->ClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void DKRenderer::ClearDepthBuffer(void) const
{
	DKRenderState* state = this->Bind();
	if (state)
	{
		state->DepthMask(true);        // required to clean depth-buffer
		glClear(GL_DEPTH_BUFFER_BIT);
	}
}

void DKRenderer::RenderPrimitive(DKPrimitive::Type p, const Vertex2D* vertices, size_t count, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (vertices == NULL || count == 0)
		return;

	if (IsDrawable() && this->Bind())
	{
		RendererContext* ctxt = GetContext();
		RendererContext::CriticalSection section(ctxt->lock);

		ctxt->buffer.Clear();
		ctxt->buffer.Reserve(count);

		for (size_t i = 0; i < count; i++)
		{
			ctxt->buffer.Add(Vertex2D(DKVector2(vertices[i].position.x, vertices[i].position.y).Transform(screenTM), vertices[i].texcoord));
		}
		ctxt->Update2DMeshStream(p, ctxt->buffer, ctxt->buffer.Count());
		ctxt->mesh2D->SetMaterialProperty(L"color", DKMaterial::PropertyArray(color.val, 4));

		if (texture && texture->IsValid())
		{
			ctxt->mesh2D->SetSampler(L"tex", const_cast<DKTexture*>(texture), const_cast<DKTextureSampler*>(sampler));
			ctxt->sceneState.sceneIndex = Private::RP2Textured;
		}
		else
		{
			ctxt->sceneState.sceneIndex = Private::RP2Colored;
		}
		this->RenderMesh(ctxt->mesh2D, ctxt->sceneState, &blend);
		ctxt->mesh2D->RemoveSampler(L"tex");
		ctxt->buffer.Clear();
	}
}

void DKRenderer::RenderPrimitive(DKPrimitive::Type p, const Vertex3D* vertices, size_t count, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKBlendState& blend) const
{
	if (vertices == NULL || count == 0)
		return;

	if (IsDrawable() && this->Bind())
	{
		RendererContext* ctxt = GetContext();
		RendererContext::CriticalSection section(ctxt->lock);
		ctxt->Update3DMeshStream(p, vertices, count);
		ctxt->mesh3D->SetMaterialProperty(L"transform", DKMaterial::PropertyArray(tm.val, 16));

		if (texture && texture->IsValid())
		{
			ctxt->mesh3D->SetSampler(L"tex", const_cast<DKTexture*>(texture), const_cast<DKTextureSampler*>(sampler));
			ctxt->sceneState.sceneIndex = Private::RP3Textured;
		}
		else
		{
			ctxt->sceneState.sceneIndex = Private::RP3Colored;
		}
		this->RenderMesh(ctxt->mesh3D, ctxt->sceneState, &blend);
		ctxt->mesh3D->RemoveSampler(L"tex");
	}
}

void DKRenderer::RenderSolidRect(const DKRect& rect, const DKMatrix3& transform, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && rect.IsValid())
	{
		const DKVector2 pos[4] = {
			DKVector2(rect.origin.x, rect.origin.y).Transform(transform),											// left-bottom
			DKVector2(rect.origin.x, rect.origin.y + rect.size.height).Transform(transform),						// left-top
			DKVector2(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height).Transform(transform),		// right-top
			DKVector2(rect.origin.x + rect.size.width, rect.origin.y).Transform(transform)							// right-bottom
		};

		const DKVector2 tpos[4] = {
			DKVector2(pos[0]).Transform(this->contentTM),
			DKVector2(pos[1]).Transform(this->contentTM),
			DKVector2(pos[2]).Transform(this->contentTM),
			DKVector2(pos[3]).Transform(this->contentTM),
		};

		bool t1 = this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]);
		bool t2 = this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]);
		if (t1 && t2)
		{
			const Vertex2D vf[4] = {
				Vertex2D(pos[1], DKPoint(0, 0)),
				Vertex2D(pos[0], DKPoint(0, 0)),
				Vertex2D(pos[2], DKPoint(0, 0)),
				Vertex2D(pos[3], DKPoint(0, 0))
			};
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 4, NULL, NULL, color, blend);
		}
		else if (t1)
		{
			const Vertex2D vf[3] = {
				Vertex2D(pos[1], DKPoint(0, 0)),
				Vertex2D(pos[0], DKPoint(0, 0)),
				Vertex2D(pos[2], DKPoint(0, 0))
			};
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, NULL, NULL, color, blend);
		}
		else if (t2)
		{
			const Vertex2D vf[3] = {
				Vertex2D(pos[2], DKPoint(0, 0)),
				Vertex2D(pos[0], DKPoint(0, 0)),
				Vertex2D(pos[3], DKPoint(0, 0))
			};
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, NULL, NULL, color, blend);
		}
	}
}

void DKRenderer::RenderSolidRects(const DKRect* rects, const DKMatrix3* transforms, size_t numRects, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && rects && numRects > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numRects);

		for (size_t i = 0; i < numRects; ++i)
		{
			if (rects[i].IsValid())
			{
				DKVector2 pos[4] = {
					DKVector2(rects[i].origin.x, rects[i].origin.y),												// left-bottom
					DKVector2(rects[i].origin.x, rects[i].origin.y + rects[i].size.height),							// left-top
					DKVector2(rects[i].origin.x + rects[i].size.width, rects[i].origin.y + rects[i].size.height),	// right-top
					DKVector2(rects[i].origin.x + rects[i].size.width, rects[i].origin.y)							// right-bottom			
				};

				if (transforms)
				{
					pos[0].Transform(transforms[i]);
					pos[1].Transform(transforms[i]);
					pos[2].Transform(transforms[i]);
					pos[3].Transform(transforms[i]);
				}

				const DKVector2 tpos[4] = {
					DKVector2(pos[0]).Transform(this->contentTM),
					DKVector2(pos[1]).Transform(this->contentTM),
					DKVector2(pos[2]).Transform(this->contentTM),
					DKVector2(pos[3]).Transform(this->contentTM),
				};

				if (this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]))
				{
					reusableVert2DBuffer.Add(Vertex2D(pos[1], DKPoint(0, 0)));
					reusableVert2DBuffer.Add(Vertex2D(pos[0], DKPoint(0, 0)));
					reusableVert2DBuffer.Add(Vertex2D(pos[2], DKPoint(0, 0)));
				}
				if (this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]))
				{
					reusableVert2DBuffer.Add(Vertex2D(pos[2], DKPoint(0, 0)));
					reusableVert2DBuffer.Add(Vertex2D(pos[0], DKPoint(0, 0)));
					reusableVert2DBuffer.Add(Vertex2D(pos[3], DKPoint(0, 0)));
				}
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidQuad(const DKPoint& lb, const DKPoint& lt, const DKPoint& rt, const DKPoint& rb, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable())
	{
		const DKVector2 tpos[4] = {
			lb.Vector().Transform(this->contentTM),			// left-bottom
			lt.Vector().Transform(this->contentTM),			// left-top
			rt.Vector().Transform(this->contentTM),			// right-top
			rb.Vector().Transform(this->contentTM),			// right-bottom
		};
		bool t1 = this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]);
		bool t2 = this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]);
		if (t1 && t2)
		{
			const Vertex2D vf[4] = { Vertex2D(lt, DKPoint(0, 0)), Vertex2D(lb, DKPoint(0, 1)), Vertex2D(rt, DKPoint(1, 0)), Vertex2D(rb, DKPoint(1, 1)) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 4, NULL, NULL, color, blend);
		}
		else if (t1)
		{
			const Vertex2D vf[3] = { Vertex2D(lt, DKPoint(0, 0)), Vertex2D(lb, DKPoint(0, 1)), Vertex2D(rt, DKPoint(1, 0)) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, NULL, NULL, color, blend);
		}
		else if (t2)
		{
			const Vertex2D vf[3] = { Vertex2D(rt, DKPoint(1, 0)), Vertex2D(lb, DKPoint(0, 1)), Vertex2D(rb, DKPoint(1, 1)) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, NULL, NULL, color, blend);
		}
	}
}

void DKRenderer::RenderSolidQuads(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 3)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; (i + 3) < numPoints; i += 4)
		{
			const DKVector2 tpos[4] = {
				points[i].Vector().Transform(this->contentTM),
				points[i + 1].Vector().Transform(this->contentTM),
				points[i + 2].Vector().Transform(this->contentTM),
				points[i + 3].Vector().Transform(this->contentTM)
			};
			if (this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]))
			{
				reusableVert2DBuffer.Add(Vertex2D(points[i + 1], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 0], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 2], DKPoint(0, 0)));
			}
			if (this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]))
			{
				reusableVert2DBuffer.Add(Vertex2D(points[i + 2], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 0], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 3], DKPoint(0, 0)));
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidTriangle(const DKPoint& pos1, const DKPoint& pos2, const DKPoint& pos3, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable())
	{
		const DKVector2 tpos[3] = {
			pos1.Vector().Transform(this->contentTM),
			pos2.Vector().Transform(this->contentTM),
			pos3.Vector().Transform(this->contentTM)
		};
		if (this->contentBounds.IntersectTriangle(tpos[0], tpos[1], tpos[2]))
		{
			const Vertex2D vf[3] = { Vertex2D(pos1, DKPoint(0, 0)), Vertex2D(pos2, DKPoint(0, 0)), Vertex2D(pos3, DKPoint(0, 0)) };
			RenderPrimitive(DKPrimitive::TypeTriangles, vf, 3, NULL, NULL, color, blend);
		}
	}
}

void DKRenderer::RenderSolidTriangles(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; (i + 2) < numPoints; i += 3)
		{
			const DKVector2 tpos[3] = {
				points[i].Vector().Transform(this->contentTM),
				points[i + 1].Vector().Transform(this->contentTM),
				points[i + 2].Vector().Transform(this->contentTM)
			};
			if (this->contentBounds.IntersectTriangle(tpos[0], tpos[1], tpos[2]))
			{
				reusableVert2DBuffer.Add(Vertex2D(points[i + 0], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 1], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 2], DKPoint(0, 0)));
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidTriangleStrip(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert2DBuffer.Add(Vertex2D(points[i], DKPoint(0, 0)));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleStrip, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidTriangleFan(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert2DBuffer.Add(Vertex2D(points[i], DKPoint(0, 0)));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleFan, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidLine(const DKPoint& pos1, const DKPoint& pos2, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable())
	{
		const DKVector2 tpos[2] = {
			pos1.Vector().Transform(this->contentTM),
			pos2.Vector().Transform(this->contentTM)
		};
		if (this->contentBounds.IntersectLine(tpos[0], tpos[1]))
		{
			const Vertex2D vf[2] = { Vertex2D(pos1, DKPoint(0, 0)), Vertex2D(pos2, DKPoint(0, 0)) };
			RenderPrimitive(DKPrimitive::TypeLines, vf, 2, NULL, NULL, color, blend);
		}
	}
}

void DKRenderer::RenderSolidLines(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; (i + 1) < numPoints; i += 2)
		{
			const DKVector2 tpos[2] = {
				points[i].Vector().Transform(this->contentTM),
				points[i + 1].Vector().Transform(this->contentTM)
			};
			if (this->contentBounds.IntersectLine(tpos[0], tpos[1]))
			{
				reusableVert2DBuffer.Add(Vertex2D(points[i + 0], DKPoint(0, 0)));
				reusableVert2DBuffer.Add(Vertex2D(points[i + 1], DKPoint(0, 0)));
			}
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidLineStrip(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert2DBuffer.Add(Vertex2D(points[i], DKPoint(0, 0)));
		}
		RenderPrimitive(DKPrimitive::TypeLineStrip, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidPoint(const DKPoint& point, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable())
	{
		const DKVector2 tpos = point.Vector().Transform(this->contentTM);
		if (this->contentBounds.IsPointInside(tpos))
		{
			Vertex2D vert(point, DKPoint(0, 0));
			RenderPrimitive(DKPrimitive::TypePoints, &vert, 1, NULL, NULL, color, blend);
		}
	}
}

void DKRenderer::RenderSolidPoints(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			const DKVector2 tpos = points[i].Vector().Transform(this->contentTM);
			if (this->contentBounds.IsPointInside(tpos))
				reusableVert2DBuffer.Add(Vertex2D(points[i], DKPoint(0, 0)));
		}
		RenderPrimitive(DKPrimitive::TypePoints, reusableVert2DBuffer, reusableVert2DBuffer.Count(), NULL, NULL, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidEllipse(const DKRect& bounds, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && bounds.IsValid())
	{
		DKVector2 plb = DKVector2(bounds.origin.x, bounds.origin.y).Transform(screenTM);											// left-bottom
		DKVector2 plt = DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(screenTM);						// left-top
		DKVector2 prt = DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(screenTM);	// right-top
		DKVector2 prb = DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(screenTM);						// right-bottom

		const DKRect local(-1.0f, -1.0f, 2.0f, 2.0f); // 3d frustum space of screen.
		if (local.IntersectTriangle(plb, plt, prb) || local.IntersectTriangle(prt, prb, plt))
		{
			// set 4 verts with (lt, lb, rt, rb)
			const Vertex2D vf[4] = { Vertex2D(plt, DKPoint(0, 0)), Vertex2D(plb, DKPoint(0, 0)), Vertex2D(prt, DKPoint(0, 0)), Vertex2D(prb, DKPoint(0, 0)) };

			// formula: X^2 / A^2 + Y^2 / B^2 = 1
			// A^2 = bounds.width/2, B^2 = bounds.height/2
			const DKVector2 center = DKVector2(bounds.origin.x + bounds.size.width / 2, bounds.origin.y + bounds.size.height / 2).Transform(screenTM);
			const DKVector2 radius((prb - plb).Length() / 2, (plt - plb).Length() / 2);
			const float radiusSq[2] = { radius.x * radius.x, radius.y * radius.y };

			if (this->Bind())
			{
				RendererContext* ctxt = GetContext();
				RendererContext::CriticalSection section(ctxt->lock);
				ctxt->Update2DMeshStream(DKPrimitive::TypeTriangleStrip, vf, 4);
				ctxt->mesh2D->SetMaterialProperty(L"color", DKMaterial::PropertyArray(color.val, 4));
				ctxt->mesh2D->SetMaterialProperty(L"radiusSq", DKMaterial::PropertyArray(radiusSq, 2));
				ctxt->mesh2D->SetMaterialProperty(L"center", DKMaterial::PropertyArray(center.val, 2));
				ctxt->sceneState.sceneIndex = Private::RP2SolidEllipse;
				this->RenderMesh(ctxt->mesh2D, ctxt->sceneState, &blend);
			}
		}
	}
}

void DKRenderer::RenderTexturedRect(const DKRect& posRect, const DKMatrix3& posTM, const DKRect& texRect, const DKMatrix3& texTM, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture && posRect.IsValid())
	{
		const DKVector2 pos[4] = {
			DKVector2(posRect.origin.x, posRect.origin.y).Transform(posTM),												// left-bottom
			DKVector2(posRect.origin.x, posRect.origin.y + posRect.size.height).Transform(posTM),						// left-top
			DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y + posRect.size.height).Transform(posTM),	// right-top
			DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y).Transform(posTM)							// right-bottom
		};
		const DKVector2 tex[4] = {
			DKVector2(texRect.origin.x, texRect.origin.y).Transform(texTM),												// left-bottom
			DKVector2(texRect.origin.x, texRect.origin.y + texRect.size.height).Transform(texTM),						// left-top
			DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y + texRect.size.height).Transform(texTM),	// right-top
			DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y).Transform(texTM)							// right-bottom
		};

		const DKVector2 tpos[4] = {
			DKVector2(pos[0]).Transform(this->contentTM),
			DKVector2(pos[1]).Transform(this->contentTM),
			DKVector2(pos[2]).Transform(this->contentTM),
			DKVector2(pos[3]).Transform(this->contentTM),
		};

		bool t1 = this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]);
		bool t2 = this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]);
		if (t1 && t2)
		{
			const Vertex2D vf[4] = { Vertex2D(pos[1], tex[1]), Vertex2D(pos[0], tex[0]), Vertex2D(pos[2], tex[2]), Vertex2D(pos[3], tex[3]) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 4, texture, sampler, color, blend);
		}
		else if (t1)
		{
			const Vertex2D vf[3] = { Vertex2D(pos[1], tex[1]), Vertex2D(pos[0], tex[0]), Vertex2D(pos[2], tex[2]) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, texture, sampler, color, blend);
		}
		else if (t2)
		{
			const Vertex2D vf[3] = { Vertex2D(pos[2], tex[2]), Vertex2D(pos[0], tex[0]), Vertex2D(pos[3], tex[3]) };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, texture, sampler, color, blend);
		}
	}
}

void DKRenderer::RenderTexturedRects(const DKRect* posRects, const DKMatrix3* posTMs, const DKRect* texRects, const DKMatrix3* texTMs, size_t numRects, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && posRects && texRects && texture && numRects > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numRects * 6);

		for (size_t i = 0; i < numRects; ++i)
		{
			if (posRects[i].IsValid())
			{
				DKVector2 pos[4] = {
					DKVector2(posRects[i].origin.x, posRects[i].origin.y),															// left-bottom
					DKVector2(posRects[i].origin.x, posRects[i].origin.y + posRects[i].size.height),								// left-top
					DKVector2(posRects[i].origin.x + posRects[i].size.width, posRects[i].origin.y + posRects[i].size.height),		// right-top
					DKVector2(posRects[i].origin.x + posRects[i].size.width, posRects[i].origin.y)									// right-bottom			
				};
				DKVector2 tex[4] = {
					DKVector2(texRects[i].origin.x, texRects[i].origin.y),															// left-bottom
					DKVector2(texRects[i].origin.x, texRects[i].origin.y + texRects[i].size.height),								// left-top
					DKVector2(texRects[i].origin.x + texRects[i].size.width, texRects[i].origin.y + texRects[i].size.height),		// right-top
					DKVector2(texRects[i].origin.x + texRects[i].size.width, texRects[i].origin.y)									// right-bottom
				};

				if (posTMs)
				{
					pos[0].Transform(posTMs[i]);
					pos[1].Transform(posTMs[i]);
					pos[2].Transform(posTMs[i]);
					pos[3].Transform(posTMs[i]);
				}
				if (texTMs)
				{
					tex[0].Transform(texTMs[i]);
					tex[1].Transform(texTMs[i]);
					tex[2].Transform(texTMs[i]);
					tex[3].Transform(texTMs[i]);
				}

				const DKVector2 tpos[4] = {
					DKVector2(pos[0]).Transform(this->contentTM),
					DKVector2(pos[1]).Transform(this->contentTM),
					DKVector2(pos[2]).Transform(this->contentTM),
					DKVector2(pos[3]).Transform(this->contentTM)
				};
				if (this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]))
				{
					reusableVert2DBuffer.Add(Vertex2D(pos[1], tex[1]));
					reusableVert2DBuffer.Add(Vertex2D(pos[0], tex[0]));
					reusableVert2DBuffer.Add(Vertex2D(pos[2], tex[2]));
				}
				if (this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]))
				{
					reusableVert2DBuffer.Add(Vertex2D(pos[2], tex[2]));
					reusableVert2DBuffer.Add(Vertex2D(pos[0], tex[0]));
					reusableVert2DBuffer.Add(Vertex2D(pos[3], tex[3]));
				}
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), texture, sampler, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedQuad(const Vertex2D& lb, const Vertex2D& lt, const Vertex2D& rt, const Vertex2D& rb, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture)
	{
		const DKVector2 tpos[4] = {
			lb.position.Vector().Transform(this->contentTM),
			lt.position.Vector().Transform(this->contentTM),
			rt.position.Vector().Transform(this->contentTM),
			rb.position.Vector().Transform(this->contentTM)
		};
		bool t1 = this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]);
		bool t2 = this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]);
		if (t1 && t2)
		{
			const Vertex2D vf[4] = { lt, lb, rt, rb };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 4, texture, sampler, color, blend);
		}
		else if (t1)
		{
			const Vertex2D vf[3] = { lt, lb, rt };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, texture, sampler, color, blend);
		}
		else if (t2)
		{
			const Vertex2D vf[3] = { rt, lb, rb };
			RenderPrimitive(DKPrimitive::TypeTriangleStrip, vf, 3, texture, sampler, color, blend);
		}
	}
}

void DKRenderer::RenderTexturedQuads(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 3)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numVerts);

		for (size_t i = 0; (i + 3) < numVerts; i += 4)
		{
			const DKVector2 tpos[4] = {
				vertices[i].position.Vector().Transform(this->contentTM),
				vertices[i + 1].position.Vector().Transform(this->contentTM),
				vertices[i + 2].position.Vector().Transform(this->contentTM),
				vertices[i + 3].position.Vector().Transform(this->contentTM),
			};

			if (this->contentBounds.IntersectTriangle(tpos[1], tpos[0], tpos[2]))
			{
				reusableVert2DBuffer.Add(vertices[i + 1]);
				reusableVert2DBuffer.Add(vertices[i + 0]);
				reusableVert2DBuffer.Add(vertices[i + 2]);
			}
			if (this->contentBounds.IntersectTriangle(tpos[2], tpos[0], tpos[3]))
			{
				reusableVert2DBuffer.Add(vertices[i + 2]);
				reusableVert2DBuffer.Add(vertices[i + 0]);
				reusableVert2DBuffer.Add(vertices[i + 3]);
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), texture, sampler, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedTriangle(const Vertex2D& v1, const Vertex2D& v2, const Vertex2D& v3, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture)
	{
		const DKVector2 tpos[3] = {
			v1.position.Vector().Transform(this->contentTM),
			v2.position.Vector().Transform(this->contentTM),
			v3.position.Vector().Transform(this->contentTM)
		};
		if (this->contentBounds.IntersectTriangle(tpos[0], tpos[1], tpos[2]))
		{
			const Vertex2D vf[3] = { v1, v2, v3 };
			RenderPrimitive(DKPrimitive::TypeTriangles, vf, 3, texture, sampler, color, blend);
		}
	}
}

void DKRenderer::RenderTexturedTriangles(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numVerts);

		for (size_t i = 0; (i + 2) < numVerts; i += 3)
		{
			const DKVector2 tpos[3] = {
				vertices[i + 0].position.Vector().Transform(this->contentTM),
				vertices[i + 1].position.Vector().Transform(this->contentTM),
				vertices[i + 2].position.Vector().Transform(this->contentTM)
			};
			if (this->contentBounds.IntersectTriangle(tpos[0], tpos[1], tpos[2]))
			{
				reusableVert2DBuffer.Add(vertices[i + 0]);
				reusableVert2DBuffer.Add(vertices[i + 1]);
				reusableVert2DBuffer.Add(vertices[i + 2]);
			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert2DBuffer, reusableVert2DBuffer.Count(), texture, sampler, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedTriangleStrip(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		RenderPrimitive(DKPrimitive::TypeTriangleStrip, vertices, numVerts, texture, sampler, color, blend);
	}
}

void DKRenderer::RenderTexturedTriangleFan(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		RenderPrimitive(DKPrimitive::TypeTriangleFan, vertices, numVerts, texture, sampler, color, blend);
	}
}

void DKRenderer::RenderTexturedLine(const Vertex2D& v1, const Vertex2D& v2, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture)
	{
		const DKVector2 tpos[2] = {
			v1.position.Vector().Transform(this->contentTM),
			v2.position.Vector().Transform(this->contentTM)
		};
		if (this->contentBounds.IntersectLine(tpos[0], tpos[1]))
		{
			const Vertex2D vf[2] = { v1, v2 };
			RenderPrimitive(DKPrimitive::TypeLines, vf, 2, texture, sampler, color, blend);
		}
	}
}

void DKRenderer::RenderTexturedLines(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numVerts);

		for (size_t i = 0; (i + 1) < numVerts; i += 2)
		{
			const DKVector2 tpos[2] = {
				vertices[i + 0].position.Vector().Transform(this->contentTM),
				vertices[i + 1].position.Vector().Transform(this->contentTM)
			};
			if (this->contentBounds.IntersectLine(tpos[0], tpos[1]))
			{
				reusableVert2DBuffer.Add(vertices[i + 0]);
				reusableVert2DBuffer.Add(vertices[i + 1]);
			}
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert2DBuffer, reusableVert2DBuffer.Count(), texture, sampler, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedLineStrip(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 1)
	{
		RenderPrimitive(DKPrimitive::TypeLineStrip, vertices, numVerts, texture, sampler, color, blend);
	}
}

void DKRenderer::RenderTexturedPoint(const Vertex2D& vertex, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture)
	{
		const DKVector2 tpos = vertex.position.Vector().Transform(this->contentTM);
		if (this->contentBounds.IsPointInside(tpos))
		{
			RenderPrimitive(DKPrimitive::TypePoints, &vertex, 1, texture, sampler, color, blend);
		}
	}
}

void DKRenderer::RenderTexturedPoints(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert2DBuffer.Clear();
		reusableVert2DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			const DKVector2 tpos = vertices[i].position.Vector().Transform(this->contentTM);
			if (this->contentBounds.IsPointInside(tpos))
				reusableVert2DBuffer.Add(vertices[i]);
		}
		RenderPrimitive(DKPrimitive::TypePoints, reusableVert2DBuffer, reusableVert2DBuffer.Count(), texture, sampler, color, blend);
		reusableVert2DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedEllipse(const DKRect& bounds, const DKRect& texBounds, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && texture && bounds.IsValid())
	{
		DKVector2 plb = DKVector2(bounds.origin.x, bounds.origin.y).Transform(screenTM);											// left-bottom
		DKVector2 plt = DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(screenTM);						// left-top
		DKVector2 prt = DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(screenTM);	// right-top
		DKVector2 prb = DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(screenTM);						// right-bottom

		const DKRect local(-1.0f, -1.0f, 2.0f, 2.0f); // 3d frustum space of screen.
		if (local.IntersectTriangle(plb, plt, prb) || local.IntersectTriangle(prt, prb, plt))
		{
			DKPoint tlb(texBounds.origin);																		// left-bottom
			DKPoint tlt(texBounds.origin.x, texBounds.origin.y + texBounds.size.height);						// left-top
			DKPoint trt(texBounds.origin.x + texBounds.size.width, texBounds.origin.y + texBounds.size.height);	// right-top
			DKPoint trb(texBounds.origin.x + texBounds.size.width, texBounds.origin.y);							// right-bottom

			// set 4 verts with (lt, lb, rt, rb)
			const Vertex2D vf[4] = { Vertex2D(plt, tlt), Vertex2D(plb, tlb), Vertex2D(prt, trt), Vertex2D(prb, trb) };

			// formula: X^2 / A^2 + Y^2 / B^2 = 1
			// A^2 = bounds.width/2, B^2 = bounds.height/2
			const DKVector2 center = DKVector2(bounds.origin.x + bounds.size.width / 2, bounds.origin.y + bounds.size.height / 2).Transform(screenTM);
			const DKVector2 radius((prb - plb).Length() / 2, (plt - plb).Length() / 2);
			const float radiusSq[2] = { radius.x * radius.x, radius.y * radius.y };

			if (this->Bind())
			{
				RendererContext* ctxt = GetContext();
				RendererContext::CriticalSection section(ctxt->lock);

				ctxt->Update2DMeshStream(DKPrimitive::TypeTriangleStrip, vf, 4);
				ctxt->mesh2D->SetMaterialProperty(L"color", DKMaterial::PropertyArray(color.val, 4));
				ctxt->mesh2D->SetMaterialProperty(L"radiusSq", DKMaterial::PropertyArray(radiusSq, 2));
				ctxt->mesh2D->SetMaterialProperty(L"center", DKMaterial::PropertyArray(center.val, 2));

				ctxt->mesh2D->SetSampler(L"tex", const_cast<DKTexture*>(texture), const_cast<DKTextureSampler*>(sampler));
				ctxt->sceneState.sceneIndex = Private::RP2TexturedEllipse;
				this->RenderMesh(ctxt->mesh2D, ctxt->sceneState, &blend);
				ctxt->mesh2D->RemoveSampler(L"tex");
			}
		}
	}
}

void DKRenderer::RenderSolidTriangles(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	numPoints -= numPoints % 3;
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidTriangleStrip(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidTriangleFan(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleFan, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidLines(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	numPoints -= numPoints % 2;

	if (IsDrawable() && points && numPoints > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidLineStrip(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypeLineStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidPoints(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && points && numPoints > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numPoints);

		for (size_t i = 0; i < numPoints; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(points[i], DKPoint(0, 0), color));
		}
		RenderPrimitive(DKPrimitive::TypePoints, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredTriangles(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	numVerts -= numVerts % 3;

	if (IsDrawable() && vertices && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredTriangleStrip(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredTriangleFan(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleFan, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredLines(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	numVerts -= numVerts % 2;

	if (IsDrawable() && vertices && numVerts > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredLineStrip(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && numVerts > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypeLineStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderColoredPoints(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && numVerts > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, DKPoint(0, 0), vertices[i].color));
		}
		RenderPrimitive(DKPrimitive::TypePoints, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedTriangles(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	numVerts -= numVerts % 3;

	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedTriangleStrip(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedTriangleFan(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypeTriangleFan, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedLines(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	numVerts -= numVerts % 2;

	if (IsDrawable() && vertices && texture && numVerts > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedLineStrip(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 1)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypeLineStrip, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderTexturedPoints(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() && vertices && texture && numVerts > 0)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(numVerts);

		for (size_t i = 0; i < numVerts; ++i)
		{
			reusableVert3DBuffer.Add(Vertex3D(vertices[i].position, vertices[i].texcoord, color));
		}
		RenderPrimitive(DKPrimitive::TypePoints, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, texture, sampler, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidSphere(const DKVector3& center, float radius, int lats, int longs, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (radius > 0 && lats > 1 && longs > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(lats * longs * 6);

		const DKPoint texCoord(0, 0);

		for (int i = 0; i < lats; ++i)
		{
			float lat0 = DKL_PI * (static_cast<float>(i) / lats - 0.5f);
			float lat1 = DKL_PI * (static_cast<float>(i + 1) / lats - 0.5f);
			float y0 = radius * sin(lat0);
			float y1 = radius * sin(lat1);
			float r0 = radius * cos(lat0);
			float r1 = radius * cos(lat1);

			for (int j = 0; j < longs; ++j)
			{
				float lng0 = 2 * DKL_PI * static_cast<float>(j) / longs;
				float lng1 = 2 * DKL_PI * static_cast<float>(j + 1) / longs;
				float x0 = cos(lng0);
				float x1 = cos(lng1);
				float z0 = sin(lng0);
				float z1 = sin(lng1);

				DKVector3 quad[4] = {
					DKVector3(x0 * r0, y0, z0 * r0) + center,
					DKVector3(x0 * r1, y1, z0 * r1) + center,
					DKVector3(x1 * r0, y0, z1 * r0) + center,
					DKVector3(x1 * r1, y1, z1 * r1) + center
				};

				reusableVert3DBuffer.Add(Vertex3D(quad[0], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[1], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[2], texCoord, color));

				reusableVert3DBuffer.Add(Vertex3D(quad[2], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[1], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[3], texCoord, color));

			}
		}
		RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderWireSphere(const DKVector3& center, float radius, int lats, int longs, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (radius > 0 && lats > 1 && longs > 2)
	{
		DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
		reusableVert3DBuffer.Clear();
		reusableVert3DBuffer.Reserve(lats * longs * 4);

		const DKPoint texCoord(0, 0);

		for (int i = 0; i < lats; ++i)
		{
			float lat0 = DKL_PI * (static_cast<float>(i) / lats - 0.5f);
			float lat1 = DKL_PI * (static_cast<float>(i + 1) / lats - 0.5f);
			float y0 = radius * sin(lat0);
			float y1 = radius * sin(lat1);
			float r0 = radius * cos(lat0);
			float r1 = radius * cos(lat1);

			for (int j = 0; j < longs; ++j)
			{
				float lng0 = 2 * DKL_PI * static_cast<float>(j) / longs;
				float lng1 = 2 * DKL_PI * static_cast<float>(j + 1) / longs;
				float x0 = cos(lng0);
				float x1 = cos(lng1);
				float z0 = sin(lng0);
				float z1 = sin(lng1);

				DKVector3 quad[4] = {
					DKVector3(x0 * r0, y0, z0 * r0) + center,
					DKVector3(x0 * r1, y1, z0 * r1) + center,
					DKVector3(x1 * r0, y0, z1 * r0) + center,
					DKVector3(x1 * r1, y1, z1 * r1) + center
				};

				reusableVert3DBuffer.Add(Vertex3D(quad[0], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[1], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[0], texCoord, color));
				reusableVert3DBuffer.Add(Vertex3D(quad[2], texCoord, color));
			}
		}
		RenderPrimitive(DKPrimitive::TypeLines, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
		reusableVert3DBuffer.Clear();
	}
}

void DKRenderer::RenderSolidAABB(const DKVector3& aabbMin, const DKVector3& aabbMax, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (aabbMax.x < aabbMin.x || aabbMax.y < aabbMin.y || aabbMax.z < aabbMin.z)
		return;

	DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
	reusableVert3DBuffer.Clear();
	reusableVert3DBuffer.Reserve(36);

	const DKPoint texCoord(0, 0);

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));

	RenderPrimitive(DKPrimitive::TypeTriangles, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
	reusableVert3DBuffer.Clear();
}

void DKRenderer::RenderWireAABB(const DKVector3& aabbMin, const DKVector3& aabbMax, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend) const
{
	if (aabbMax.x < aabbMin.x || aabbMax.y < aabbMin.y || aabbMax.z < aabbMin.z)
		return;

	DKCriticalSection<DKSpinLock> guard(reusableBufferSpinLock);
	reusableVert3DBuffer.Clear();
	reusableVert3DBuffer.Reserve(12);

	const DKPoint texCoord(0, 0);

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));

	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMin.x, aabbMax.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMin.y, aabbMax.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMin.z), texCoord, color));
	reusableVert3DBuffer.Add(Vertex3D(DKVector3(aabbMax.x, aabbMax.y, aabbMax.z), texCoord, color));

	RenderPrimitive(DKPrimitive::TypeLines, reusableVert3DBuffer, reusableVert3DBuffer.Count(), tm, NULL, NULL, blend);
	reusableVert3DBuffer.Clear();
}

void DKRenderer::RenderText(const DKRect& bounds, const DKMatrix3& transform, const DKFoundation::DKString& text, const DKFont* font, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() == false)
		return;
	if (bounds.size.width == 0 || bounds.size.height == 0)
		return;

	size_t textLen = text.Length();
	if (font == NULL || !font->IsValid() || textLen == 0)
		return;

	struct TextureQuad
	{
		DKRenderer::Vertex2D		bottomLeft;
		DKRenderer::Vertex2D		topLeft;
		DKRenderer::Vertex2D		bottomRight;
		DKRenderer::Vertex2D		topRight;
		const DKTexture*		texture;
		static bool OrderByTextureASC(const TextureQuad& lhs, const TextureQuad& rhs)
		{
			return reinterpret_cast<const char*>(lhs.texture) > reinterpret_cast<const char*>(rhs.texture);
		}
	};

	DKArray<TextureQuad>	quads;
	quads.Reserve(textLen);

	DKPoint bboxMin(0, 0);
	DKPoint bboxMax(0, 0);
	float offset = 0;			// accumulated text width (pixel)

	for (size_t i = 0; i < textLen; i++)
	{
		// get glyph info from font object
		const DKFont::GlyphData* glyph = font->GlyphDataForChar(text[i]);
		if (glyph == NULL)
			continue;

		DKPoint posMin(offset + glyph->position.x, glyph->position.y);
		DKPoint posMax(offset + glyph->position.x + glyph->rect.size.width, glyph->position.y + glyph->rect.size.height);

		if (bboxMin.x > posMin.x)	bboxMin.x = posMin.x;
		if (bboxMin.y > posMin.y)	bboxMin.y = posMin.y;
		if (bboxMax.x < posMax.x)	bboxMax.x = posMax.x;
		if (bboxMax.y < posMax.y)	bboxMax.y = posMax.y;

		if (glyph->texture)
		{
			DKSize textureSize = glyph->texture->Resolution();
			if (textureSize.width > 0 && textureSize.height > 0)
			{
				DKPoint uvMin(glyph->rect.origin.x / textureSize.width, glyph->rect.origin.y / textureSize.height);
				DKPoint uvMax((glyph->rect.origin.x + glyph->rect.size.width) / textureSize.width, (glyph->rect.origin.y + glyph->rect.size.height) / textureSize.height);

				const TextureQuad q =
				{
					Vertex2D(DKPoint(posMin.x, posMin.y), DKPoint(uvMin.x, uvMin.y)),
					Vertex2D(DKPoint(posMin.x, posMax.y), DKPoint(uvMin.x, uvMax.y)),
					Vertex2D(DKPoint(posMax.x, posMin.y), DKPoint(uvMax.x, uvMin.y)),
					Vertex2D(DKPoint(posMax.x, posMax.y), DKPoint(uvMax.x, uvMax.y)),
					glyph->texture,
				};
				quads.Add(q);
			}
		}
		offset += glyph->advance.width + font->KernAdvance(text[i], text[i + 1]).x;
	}
	if (quads.IsEmpty())
		return;

	const float width = bboxMax.x - bboxMin.x;
	const float height = bboxMax.y - bboxMin.y;

	if (width <= 0.0 || height <= 0.0)
		return;

	// calculate transform (matrix)
	DKAffineTransform2 trans;
	trans.Translate(-bboxMin.x, -bboxMin.y);									// move origin
	trans *= DKLinearTransform2().Scale(1.0 / width, 1.0 / height);				// normalize size
	trans *= DKLinearTransform2().Scale(bounds.size.width, bounds.size.height);	// set scale to bounds
	trans.Translate(bounds.origin.x, bounds.origin.y);							// move to bounds.origin

	DKMatrix3 matrix = trans.Matrix3();
	matrix *= transform;								// user's transform
	matrix *= screenTM;									// transform to screen-space

	for (size_t i = 0; i < quads.Count(); i++)
	{
		TextureQuad& q = quads.Value(i);
		q.bottomLeft.position = q.bottomLeft.position.Vector().Transform(matrix);
		q.bottomRight.position = q.bottomRight.position.Vector().Transform(matrix);
		q.topLeft.position = q.topLeft.position.Vector().Transform(matrix);
		q.topRight.position = q.topRight.position.Vector().Transform(matrix);
	}
	// sort by texture (same texture first)
	quads.Sort(0, quads.Count(), TextureQuad::OrderByTextureASC);

	if (this->Bind())
	{
		RendererContext* ctxt = GetContext();
		RendererContext::CriticalSection section(ctxt->lock);

		ctxt->buffer.Clear();
		ctxt->buffer.Reserve(quads.Count() * 6);  // 6 verts, (2 triangles)

		ctxt->mesh2D->SetMaterialProperty(L"color", DKMaterial::PropertyArray(color.val, 4));

		size_t beginIndex = 0;
		while (beginIndex < quads.Count())
		{
			size_t nextIndex = beginIndex;
			const DKTexture* currentTexture = quads.Value(beginIndex).texture;

			ctxt->buffer.Clear();

			while (nextIndex < quads.Count() && quads.Value(nextIndex).texture == currentTexture)
			{
				ctxt->buffer.Add(quads.Value(nextIndex).topLeft);
				ctxt->buffer.Add(quads.Value(nextIndex).bottomLeft);
				ctxt->buffer.Add(quads.Value(nextIndex).topRight);

				ctxt->buffer.Add(quads.Value(nextIndex).topRight);
				ctxt->buffer.Add(quads.Value(nextIndex).bottomLeft);
				ctxt->buffer.Add(quads.Value(nextIndex).bottomRight);

				nextIndex++;
			}

			long count = (long)nextIndex - (long)beginIndex;
			if (count > 0)
			{
				// use triangle primitive. (each triangles apart)
				ctxt->Update2DMeshStream(DKPrimitive::TypeTriangles, ctxt->buffer, ctxt->buffer.Count());
				ctxt->mesh2D->SetSampler(L"tex", const_cast<DKTexture*>(currentTexture), NULL);
				ctxt->sceneState.sceneIndex = Private::RP2AlphaTextured;
				this->RenderMesh(ctxt->mesh2D, ctxt->sceneState, &blend);
				ctxt->mesh2D->RemoveSampler(L"tex");
			}
			beginIndex = nextIndex;
		}

		ctxt->buffer.Clear();
	}
}

void DKRenderer::RenderText(const DKPoint& baselineBegin, const DKPoint& baselineEnd, const DKFoundation::DKString& text, const DKFont* font, const DKColor& color, const DKBlendState& blend) const
{
	if (IsDrawable() == false)
		return;
	if (font == NULL || !font->IsValid() || text.Length() == 0)
		return;
	if ((baselineEnd.Vector() - baselineBegin.Vector()).Length() < FLT_EPSILON)
		return;

	// font size, screen size in pixel units
	const float baseline = font->Baseline();
	const float lineHeight = font->LineHeight();
	const float lineWidth = font->LineWidth(text);
	const DKRect textBounds = font->Bounds(text);
	//const DKSize screenSize = renderTarget->Resolution();
	const DKSize& viewportSize = this->viewport.size;
	const DKSize& contentScale = this->contentBounds.size;
	//	const float aspectRatio = screenSize.width / screenSize.height;

	// change local-coords to pixel-coords
	const DKSize scaleToScreen = DKSize(viewportSize.width / contentScale.width, viewportSize.height / contentScale.height);
	const DKVector2 baselinePixelBegin = DKVector2(baselineBegin.x * scaleToScreen.width, baselineBegin.y * scaleToScreen.height);
	const DKVector2 baselinePixelEnd = DKVector2(baselineEnd.x * scaleToScreen.width, baselineEnd.y * scaleToScreen.height);
	const double scale = (baselinePixelEnd - baselinePixelBegin).Length();
	const DKVector2 baselinePixelDir = (baselinePixelEnd - baselinePixelBegin).Normalize();
	const double angle = acos(baselinePixelDir.x) * ((baselinePixelDir.y < 0) ? -1.0 : 1.0);

	// calculate transform (matrix)
	DKAffineTransform2 transform(
		DKLinearTransform2()
		.Scale(scale / lineWidth)										// scale
		.Rotate(angle)													// rotate
		.Scale(1.0 / viewportSize.width, 1.0 / viewportSize.height)		// normalize (0~1)
		.Scale(contentScale.width, contentScale.height)					// apply contentScale
		, baselineBegin.Vector());

	RenderText(textBounds, transform.Matrix3(), text, font, color, blend);
}

size_t DKRenderer::RenderMesh(const DKMesh* mesh, DKSceneState& st, const DKBlendState* blend) const
{
	size_t numInstancesDrawn = 0;

	if (mesh && IsDrawable() && this->Bind())
	{
		st.ClearModel();
		DKMesh::RenderInfo info;
		if (mesh->Bind(st, &info, blend))
		{
			if (info.numVerts > 0 && info.numInstances > 0)
			{
				//DKRenderState& state = DKOpenGLContext::RenderState();

				GLenum primType = 0;
				switch (info.primitive)
				{
				case DKPrimitive::TypePoints:					primType = GL_POINTS;					break;
				case DKPrimitive::TypeLines:					primType = GL_LINES;					break;
				case DKPrimitive::TypeLineStrip:				primType = GL_LINE_STRIP;				break;
				case DKPrimitive::TypeLineLoop:					primType = GL_LINE_LOOP;				break;
				case DKPrimitive::TypeTriangles:				primType = GL_TRIANGLES;				break;
				case DKPrimitive::TypeTriangleStrip:			primType = GL_TRIANGLE_STRIP;			break;
				case DKPrimitive::TypeTriangleFan:				primType = GL_TRIANGLE_FAN;				break;
#ifdef GL_LINES_ADJACENCY
				case DKPrimitive::TypeLinesAdjacency:			primType = GL_LINES_ADJACENCY;			break;
#endif
#ifdef GL_LINE_STRIP_ADJACENCY
				case DKPrimitive::TypeLineStripAdjacency:		primType = GL_LINE_STRIP_ADJACENCY;		break;
#endif
#ifdef GL_TRIANGLES_ADJACENCY
				case DKPrimitive::TypeTrianglesAdjacency:		primType = GL_TRIANGLES_ADJACENCY;		break;
#endif
#ifdef GL_TRIANGLE_STRIP_ADJACENCY
				case DKPrimitive::TypeTriangleStripAdjacency:	primType = GL_TRIANGLE_STRIP_ADJACENCY;	break;
#endif
				default:
					DKLog("Warning: Primitive (%x) invalid or not supported.\n", info.primitive);
					return 0;
				}

#ifdef GL_VERSION_3_1
				if (info.numIndices > 0)
				{
					auto drawCall = [](GLenum prim, GLsizei indices, GLenum type, GLsizei instances) -> GLsizei
					{
						if (instances > 1)
							glDrawElementsInstanced(prim, indices, type, 0, instances);
						else
							glDrawElements(prim, indices, type, 0);
						return instances;
					};

					switch (info.indexType)
					{
					case DKIndexBuffer::TypeUByte:
						numInstancesDrawn = drawCall(primType, info.numIndices, GL_UNSIGNED_BYTE, info.numInstances);
						break;
					case DKIndexBuffer::TypeUShort:
						numInstancesDrawn = drawCall(primType, info.numIndices, GL_UNSIGNED_SHORT, info.numInstances);
						break;
					case DKIndexBuffer::TypeUInt:
						numInstancesDrawn = drawCall(primType, info.numIndices, GL_UNSIGNED_INT, info.numInstances);
						break;
					}
				}
				else
				{
					if (info.numInstances > 1)
						glDrawArraysInstanced(primType, 0, info.numVerts, info.numInstances);
					else
						glDrawArrays(primType, 0, info.numVerts);
					numInstancesDrawn = info.numInstances;
				}
#else
				if (info.numInstances > 1)
					DKLog("Warning: instancing not supported!\n");

				if (info.numIndices > 0)
				{
					switch (info.indexType)
					{
					case DKIndexBuffer::TypeUByte:
						glDrawElements(primType, info.numIndices, GL_UNSIGNED_BYTE, 0);
						numInstancesDrawn = 1;
						break;
					case DKIndexBuffer::TypeUShort:
						glDrawElements(primType, info.numIndices, GL_UNSIGNED_SHORT, 0);
						numInstancesDrawn = 1;
						break;
					case DKIndexBuffer::TypeUInt:
						glDrawElements(primType, info.numIndices, GL_UNSIGNED_INT, 0);
						numInstancesDrawn = 1;
						break;
					}
				}
				else
				{
					glDrawArrays(primType, 0, info.numVerts);
					numInstancesDrawn = 1;
				}
#endif
			}
		}
	}
	return numInstancesDrawn;
}

void DKRenderer::RenderScene(const DKScene* scene, const DKCamera& camera, int sceneIndex, bool enableCulling, RenderSceneCallback* sc) const
{
	if (scene && IsDrawable() && this->Bind())
	{
		struct Callback : public DKScene::DrawCallback
		{
			const DKRenderer* renderer;
			RenderSceneCallback::MaterialCallback* material;
			RenderSceneCallback::ObjectColorCallback* objectColors;
			RenderSceneCallback::MeshFilter* filter;

			void DrawTriangles(VertexArray& va, const DKMatrix4& tm) override
			{
				renderer->RenderColoredTriangles(va, va.Count(), tm);
			}
			void DrawLines(VertexArray& va, const DKMatrix4& tm) override
			{
				renderer->RenderColoredLines(va, va.Count(), tm);
			}
			void DrawPoints(VertexArray& va, const DKMatrix4& tm) override
			{
				renderer->RenderColoredPoints(va, va.Count(), tm);
			}
			void DrawMeshes(MeshArray& meshes, DKSceneState& st) override
			{
				if (filter)
					filter->Invoke(meshes);

				st.userMaterialPropertyCallback = material;

				for (const DKMesh* m : meshes)
					renderer->RenderMesh(m, st);
			}
			bool ObjectColors(const DKCollisionObject* co, DKColor& fc, DKColor& ec) override
			{
				fc = DKColor(0.5f, 0.5f, 0.5f);
				ec = DKColor(0.0f, 0.0f, 0.0f);

				btCollisionObject* col = BulletCollisionObject(co);
				DKASSERT_DEBUG(col);
				switch (col->getActivationState())
				{
				case ACTIVE_TAG:			fc = DKColor(1.0f, 0.0f, 0.0f);	break;
				case ISLAND_SLEEPING:		fc = DKColor(0.5f, 0.5f, 0.5f);	break;
				case WANTS_DEACTIVATION:	fc = DKColor(0.0f, 1.0f, 0.0f);	break;
				case DISABLE_DEACTIVATION:	fc = DKColor(1.0f, 0.0f, 1.0f);	break;
				case DISABLE_SIMULATION:	fc = DKColor(0.1f, 0.1f, 0.1f);	break;
				}

				if (this->objectColors)
					return objectColors->Invoke(co, fc, ec);
				return true;
			}
		} cb;
		cb.renderer = this;
		if (sc)
		{
			cb.material = sc->materialCallback;
			cb.objectColors = sc->objectColors;
			cb.filter = sc->meshFilter;
		}
		else
		{
			cb.material = NULL;
			cb.objectColors = NULL;
			cb.filter = NULL;
		}
		scene->Render(camera, sceneIndex, enableCulling, cb);
	}
}
