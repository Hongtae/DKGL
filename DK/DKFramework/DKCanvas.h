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

        DKCanvas(DKCommandBuffer*, DKTexture*, DKTexture*);
        virtual ~DKCanvas();

        void SetViewport(const DKRect&);
        void SetContentBounds(const DKRect&);
        void SetContentTransform(const DKMatrix3&);

        const DKRect& Viewport() const;
        const DKRect& ContentBounds() const;
        const DKMatrix3& ContentTransform() const;

        // clear buffer
        virtual void Clear(const DKColor& color) = 0;
        
        // drawing basic primitives
        virtual void DrawLines(const DKPoint* points,
                               size_t numPoints,
                               const DKColor& color) = 0;
        virtual void DrawTriangles(const ColoredVertex* verts,
                                   size_t numVerts) = 0;
        virtual void DrawTriangles(const TexturedVertex* vertices,
                                   size_t numVerts,
                                   const DKTexture* texture) = 0;
        // triangle, line strip
        virtual void DrawTriangles(const DKPoint* verts,
                                   size_t numVerts,
                                   const DKColor& color);
        virtual void DrawLineStrip(const DKPoint* points,
                                   size_t numPoints,
                                   const DKColor& color);
        virtual void DrawTriangleStrip(const DKPoint* verts,
                                       size_t numVerts,
                                       const DKColor& color);
        virtual void DrawTriangleStrip(const ColoredVertex* verts,
                                       size_t numVerts);
        virtual void DrawTriangleStrip(const TexturedVertex* vertices,
                                       size_t numVerts,
                                       const DKTexture* texture);
        // Quad
        virtual void DrawQuad(const DKPoint& lt,
                              const DKPoint& rt,
                              const DKPoint& lb,
                              const DKPoint& rb,
                              const DKColor& color);
        virtual void DrawQuad(const TexturedVertex& lt,
                              const TexturedVertex& rt,
                              const TexturedVertex& lb,
                              const TexturedVertex& rb,
                              const DKTexture* texture);
        // Rect
        virtual void DrawRect(const DKRect& rect,
                              const DKMatrix3& transform,
                              const DKColor& color);
        virtual void DrawRect(const DKRect& rect,
                              const DKMatrix3& transform,
                              const DKRect& textureRect,
                              const DKMatrix3& textureTransform,
                              const DKTexture* texture,
                              const DKColor& color);
        // Ellipse
        virtual void DrawEllipseLine(const DKRect& bounds,
                                     const DKMatrix3& transform,
                                     const DKColor& color);
        virtual void DrawEllipse(const DKRect& bounds,
                                 const DKMatrix3& transform,
                                 const DKColor& color);
        virtual void DrawEllipse(const DKRect& bounds,
                                 const DKMatrix3& transform,
                                 const DKRect& textureBounds,
                                 const DKMatrix3& textureTransform,
                                 const DKTexture* texture,
                                 const DKColor& color);
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


        virtual void Commit() = 0;
        virtual bool IsDrawable(void) const { return true; }

    protected:
        virtual DKMatrix3 TargetOrientation() const { return DKMatrix3::identity; }
        virtual uint32_t LineSegmentsCircumference(float radius) const;

        void UpdateTransform();

        DKObject<DKCommandBuffer> commandBuffer;
        DKObject<DKTexture> colorAttachment;
        DKObject<DKTexture> depthAttachment;
        DKObject<DKUnknown> sharedResources;

        DKRect	  viewport;
        DKRect	  contentBounds;
        DKMatrix3 contentTransform;
        DKMatrix3 screenTransform;  // for 2d scene
    };
}
