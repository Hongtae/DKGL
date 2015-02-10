//
//  File: DKRenderer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKRenderTarget.h"
#include "DKPrimitiveIndex.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKPoint.h"
#include "DKColor.h"
#include "DKCamera.h"
#include "DKBlendState.h"
#include "DKMaterial.h"

////////////////////////////////////////////////////////////////////////////////
// DKRenderer
// renderer class, render to render-target.
// provides simple 2D/3D rendering features. (includes drawing text)
// If you need to complex scene with custom shader,
// you will need to use DKScene with DKModel, DKMesh.
//
// if DKMatrix3 used as parameters, it should be affine-transform matrix.
//
// Note:
//   Coordinates space of 2d shapes, lower-left corner is origin.
//   Coordinates space of 3d shapes, center is origin,
//   (each axis range is -1.0 ~ 1.0)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKTexture;
	class DKTextureSampler;
	class DKFont;
	class DKMesh;
	class DKScene;
	class DKSceneState;
	class DKRenderState;
	class DKCollisionObject;

	class DKLIB_API DKRenderer
	{
	public:
		static const float minimumScaleFactor;
		
		DKRenderer(DKRenderTarget*);
		virtual ~DKRenderer(void);

		struct Vertex2D
		{
			Vertex2D(const DKPoint& p, const DKPoint& t) : position(p), texcoord(t) {}
			DKPoint position;		// screen space
			DKPoint texcoord;
		};
		struct Vertex3D
		{
			Vertex3D(const DKVector3& p, const DKPoint& t, const DKColor& c) : position(p), texcoord(t), color(c) {}
			DKVector3 position;		// 3D scene space
			DKPoint texcoord;
			DKColor color;
		};
		struct Vertex3DColored
		{
			Vertex3DColored(const DKVector3& p, const DKColor& c) : position(p), color(c) {}
			DKVector3 position;		// 3D scene space
			DKColor color;
		};
		struct Vertex3DTextured
		{
			Vertex3DTextured(const DKVector3& p, const DKPoint& t) : position(p), texcoord(t) {}
			DKVector3 position;		// 3D scene space
			DKPoint texcoord;
		};

		const DKRect& Viewport(void) const;
		void SetViewport(const DKRect& rc);

		const DKRect& ContentBounds(void) const;
		void SetContentBounds(const DKRect&);

		const DKMatrix3& ContentTransform(void) const;
		void SetContentTransform(const DKMatrix3& tm);

		void SetPolygonOffset(float factor, float units);  // disabled for 0, 0
		void PolygonOffset(float*) const;

		DKRenderTarget* RenderTarget(void);
		const DKRenderTarget* RenderTarget(void) const;

		void Clear(const DKColor& color) const;
		void ClearColorBuffer(const DKColor& color) const;
		void ClearDepthBuffer(void) const;

		void RenderPrimitive(DKPrimitive::Type p, const Vertex2D* vertices, size_t count, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderPrimitive(DKPrimitive::Type p, const Vertex3D* vertices, size_t count, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		
		// 2d shape with solid color
		void RenderSolidRect(const DKRect& rect, const DKMatrix3& transform, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidRects(const DKRect* rects, const DKMatrix3* transforms, size_t numRects, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidQuad(const DKPoint& lb, const DKPoint& lt, const DKPoint& rt, const DKPoint& rb, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidQuads(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangle(const DKPoint& pos1, const DKPoint& pos2, const DKPoint& pos3, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangles(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangleStrip(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangleFan(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidLine(const DKPoint& pos1, const DKPoint& pos2, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidLines(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidLineStrip(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidPoint(const DKPoint& point, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidPoints(const DKPoint* points, size_t numPoints, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidEllipse(const DKRect& bounds, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// 2d shape with textured
		void RenderTexturedRect(const DKRect& posRect, const DKMatrix3& posTM, const DKRect& texRect, const DKMatrix3& texTM, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedRects(const DKRect* posRects, const DKMatrix3* posTMs, const DKRect* texRects, const DKMatrix3* texTMs, size_t numRects, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedQuad(const Vertex2D& lb, const Vertex2D& lt, const Vertex2D& rt, const Vertex2D& rb, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedQuads(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangle(const Vertex2D& v1, const Vertex2D& v2, const Vertex2D& v3, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangles(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangleStrip(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangleFan(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedLine(const Vertex2D& v1, const Vertex2D& v2, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedLines(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedLineStrip(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedPoint(const Vertex2D& vertex, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedPoints(const Vertex2D* vertices, size_t numVerts, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedEllipse(const DKRect& bounds, const DKRect& texBounds, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// 3d shape with solid color
		void RenderSolidTriangles(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangleStrip(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidTriangleFan(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidLines(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidLineStrip(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidPoints(const DKVector3* points, size_t numPoints, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// 3d shape with colored vertices
		void RenderColoredTriangles(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderColoredTriangleStrip(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderColoredTriangleFan(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderColoredLines(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderColoredLineStrip(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderColoredPoints(const Vertex3DColored* vertices, size_t numVerts, const DKMatrix4& tm, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// 3d shape with textured
		void RenderTexturedTriangles(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangleStrip(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedTriangleFan(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedLines(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedLineStrip(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderTexturedPoints(const Vertex3DTextured* vertices, size_t numVerts, const DKMatrix4& tm, const DKTexture* texture, const DKTextureSampler* sampler, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// 3d shapes misc..
		void RenderSolidSphere(const DKVector3& center, float radius, int lats, int longs, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderWireSphere(const DKVector3& center, float radius, int lats, int longs, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderSolidAABB(const DKVector3& aabbMin, const DKVector3& aabbMax, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;
		void RenderWireAABB(const DKVector3& aabbMin, const DKVector3& aabbMax, const DKMatrix4& tm, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultOpaque) const;

		// DKMesh
		size_t RenderMesh(const DKMesh*, DKSceneState&, const DKBlendState* blend = NULL) const;

		// DKScene
		struct RenderSceneCallback
		{
			using MaterialCallback = DKMaterial::PropertyCallback;
			using ObjectColorCallback = DKFoundation::DKFunctionSignature<bool (const DKCollisionObject*, DKColor&, DKColor&)>;
			using MeshFilter = DKFoundation::DKFunctionSignature<void (DKFoundation::DKArray<const DKMesh*>&)>;

			MaterialCallback* materialCallback = NULL;
			ObjectColorCallback* objectColors = NULL;
			MeshFilter* meshFilter = NULL;
		};
		void RenderScene(const DKScene*, const DKCamera& camera, int sceneIndex, bool enableCulling = true, RenderSceneCallback* sc = NULL) const;

		// drawing text with font.
		void RenderText(const DKRect& bounds, const DKMatrix3& transform, const DKFoundation::DKString& text, const DKFont* font, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultAlpha) const;
		void RenderText(const DKPoint& baselineBegin, const DKPoint& baselineEnd, const DKFoundation::DKString& text, const DKFont* font, const DKColor& color, const DKBlendState& blend = DKBlendState::defaultAlpha) const;

	private:
		class RendererContext;
		DKFoundation::DKObject<DKRenderTarget>			renderTarget;
		DKFoundation::DKObject<DKFoundation::DKUnknown>	context;
		DKRect											contentBounds;
		DKRect											viewport;
		DKMatrix3										contentTM;
		DKMatrix3										screenTM;  // for 2d scene

		struct
		{
			float factor;
			float units;
		} polygonOffset;

		void UpdateTransform(void);
		bool IsDrawable(void) const;
		DKRenderState* Bind(void) const;
		RendererContext* GetContext(void) const;
	};
}
