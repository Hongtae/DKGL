//
//  File: DKCommonShapeDrawer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKTexture.h"
#include "DKFont.h"
#include "DKColor.h"
#include "DKVector2.h"
#include "DKRect.h"
#include "DKMatrix3.h"
#include "DKBlendState.h"
#include "DKSampler.h"
#include "DKCommandBuffer.h"


namespace DKFramework
{
    /// @brief Render simple shapes
    ///
    class DKGL_API DKCommonShapeDrawer
    {
    public:
        struct Vertex2D
        {
            DKVector2 position;		// screen space
            DKVector2 textureCoordinate;
        };
        struct Vertex3D
        {
            DKVector3 position;		// 3D scene space
            DKVector2 textureCoordinate;
            DKColor color;
        };

        DKCommonShapeDrawer(DKCommandBuffer*, DKTexture*, DKTexture*);
        virtual ~DKCommonShapeDrawer();

        const DKRect& Viewport() const;
        void SetViewport(const DKRect& rc);

        const DKRect& ContentBounds() const;
        void SetContentBounds(const DKRect&);

        const DKMatrix3& ContentTransform() const;
        void SetContentTransform(const DKMatrix3& tm);

        void Clear(const DKColor& color) const;


        // drawing text with font.
        void RenderText(const DKRect& bounds,
                        const DKMatrix3& transform,
                        const DKFoundation::DKString& text,
                        const DKFont* font,
                        const DKColor& color,
                        const DKBlendState& blend = DKBlendState::defaultAlpha) const;

        void RenderText(const DKPoint& baselineBegin,
                        const DKPoint& baselineEnd,
                        const DKFoundation::DKString& text,
                        const DKFont* font,
                        const DKColor& color,
                        const DKBlendState& blend = DKBlendState::defaultAlpha) const;

        // draw primitive (2D)
        void DrawPrimitive(DKPrimitiveType p,
                           const Vertex2D* vertices,
                           size_t count,
                           const DKTexture* texture,
                           const DKSamplerState* sampler,
                           const DKColor& color,
                           const DKBlendState& blend = DKBlendState::defaultOpaque) const;

        // draw primitive (3D)
        void DrawPrimitive(DKPrimitiveType p,
                           const Vertex3D* vertices,
                           size_t count,
                           const DKMatrix4& tm,
                           const DKTexture* texture,
                           const DKSamplerState* sampler,
                           const DKBlendState& blend = DKBlendState::defaultOpaque) const;

    protected:
        void UpdateTransform();

        DKObject<DKCommandBuffer> commandBuffer;
        DKObject<DKTexture> colorAttachment;
        DKObject<DKTexture> depthAttachment;
        DKObject<DKUnknown> sharedResources;

        DKRect contentBounds;
        DKRect viewport;
        DKMatrix3 contentTM;
        DKMatrix3 screenTM;  // for 2d scene

        float minimumScaleFactor;
    };
}
