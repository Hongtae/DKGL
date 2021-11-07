//
//  File: DKCanvas.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKTexture.h"
#include "DKFont.h"
#include "DKColor.h"
#include "DKPoint.h"
#include "DKRect.h"
#include "DKMatrix3.h"
#include "DKBlendState.h"
#include "DKSampler.h"
#include "DKCommandBuffer.h"


namespace DKFramework
{
    /// @brief Render simple shapes
    ///
    class DKGL_API DKCanvas
    {
    public:
        static const float minimumScaleFactor;

#pragma pack(push, 4)
        struct ColoredVertex
        {
            DKPoint position;
            DKColor color;
        };
        struct TexturedVertex
        {
            DKPoint position;
            DKPoint texcoord;
            DKColor color;
        };
#pragma pack(pop)

        DKCanvas(DKCommandBuffer*, DKTexture*, DKTexture* = nullptr);
        ~DKCanvas();

        void SetViewport(const DKRect&);
        void SetContentBounds(const DKRect&);
        void SetContentTransform(const DKMatrix3&);
        void SetDeviceOrientation(const DKMatrix3&);

        const DKRect& Viewport() const;
        const DKRect& ContentBounds() const;
        const DKMatrix3& ContentTransform() const;
        const DKMatrix3& DeviceOrientation() const;

        // clear buffer
        void Clear(const DKColor& color);

        // drawing basic primitives
        void DrawLines(const DKPoint* points,
                       size_t numPoints,
                       const DKColor& color,
                       const DKBlendState& blendState,
                       float lineWidth = 1.0f);

        void DrawTriangles(const ColoredVertex* verts,
                           size_t numVerts,
                           const DKBlendState& blendState);

        void DrawTriangles(const TexturedVertex* vertices,
                           size_t numVerts,
                           const DKTexture* texture,
                           const DKBlendState& blendState);

        // triangle, line strip
        void DrawTriangles(const DKPoint* verts,
                           size_t numVerts,
                           const DKColor& color,
                           const DKBlendState& blendState);

        void DrawLineStrip(const DKPoint* points,
                           size_t numPoints,
                           const DKColor& color,
                           const DKBlendState& blendState,
                           float lineWidth = 1.0f);

        void DrawTriangleStrip(const DKPoint* verts,
                               size_t numVerts,
                               const DKColor& color,
                               const DKBlendState& blendState);

        void DrawTriangleStrip(const ColoredVertex* verts,
                               size_t numVerts,
                               const DKBlendState& blendState);

        void DrawTriangleStrip(const TexturedVertex* vertices,
                               size_t numVerts,
                               const DKTexture* texture,
                               const DKBlendState& blendState);
        // Quad
        void DrawQuad(const DKPoint& lt,
                      const DKPoint& rt,
                      const DKPoint& lb,
                      const DKPoint& rb,
                      const DKColor& color,
                      const DKBlendState& blendState);

        void DrawQuad(const TexturedVertex& lt,
                      const TexturedVertex& rt,
                      const TexturedVertex& lb,
                      const TexturedVertex& rb,
                      const DKTexture* texture,
                      const DKBlendState& blendState);
        // Rect
        void DrawRect(const DKRect& rect,
                      const DKMatrix3& transform,
                      const DKColor& color,
                      const DKBlendState& blendState);

        void DrawRect(const DKRect& rect,
                      const DKMatrix3& transform,
                      const DKRect& textureRect,
                      const DKMatrix3& textureTransform,
                      const DKTexture* texture,
                      const DKColor& color,
                      const DKBlendState& blendState);
        // Ellipse
        void DrawEllipseOutline(const DKRect& bounds,
                                const DKMatrix3& transform,
                                const DKColor& color,
                                const DKBlendState& blendState,
                                const DKSize& border);

        void DrawEllipse(const DKRect& bounds,
                         const DKMatrix3& transform,
                         const DKColor& color,
                         const DKBlendState& blendState);

        void DrawEllipse(const DKRect& bounds,
                         const DKMatrix3& transform,
                         const DKRect& textureBounds,
                         const DKMatrix3& textureTransform,
                         const DKTexture* texture,
                         const DKColor& color,
                         const DKBlendState& blendState);
        // drawing text with font.
        void DrawText(const DKRect& bounds,
                      const DKMatrix3& transform,
                      const DKString& text,
                      const DKFont* font,
                      const DKColor& color);

        void DrawText(const DKPoint& baselineBegin,
                      const DKPoint& baselineEnd,
                      const DKString& text,
                      const DKFont* font,
                      const DKColor& color);

        void Commit();
        bool IsDrawable(void) const;

        static bool CachePipelineContext(DKGraphicsDeviceContext*);
    protected:

        void UpdateTransform();

        DKObject<DKCommandBuffer> commandBuffer;
        DKObject<DKTexture> colorAttachment;
        DKObject<DKTexture> depthAttachment;
        //DKObject<DKUnknown> sharedResources;

        DKRect	  viewport;
        DKRect	  contentBounds;
        DKMatrix3 contentTransform;
        DKMatrix3 screenTransform;  // for 2d scene
        DKMatrix3 deviceOrientation;

    private:
        DKObject<DKUnknown> pipelineStates;
        bool drawable;

        void EncodeDrawCommand(uint32_t materialIndex,
                               const TexturedVertex* vertices,
                               size_t numVerts,
                               const DKTexture* texture,
                               const DKBlendState& blendState,
                               void* pushConstantData,
                               size_t pushConstantDataLength);
    };
}
