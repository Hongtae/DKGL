//
//  File: DKCommonShapeDrawer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include "DKCommonShapeDrawer.h"

using namespace DKFramework;
using namespace DKFramework::Private;

DKCommonShapeDrawer::DKCommonShapeDrawer(DKCommandBuffer* cb, DKTexture* color, DKTexture* depth)
    : commandBuffer(cb)
    , colorAttachment(color)
    , depthAttachment(depth)
    , viewport(0, 0, 1, 1)
    , contentBounds(0, 0, 1, 1)
    , contentTM(DKMatrix3::identity)
    , screenTM(DKMatrix3::identity)
    , minimumScaleFactor(0.000001f)
{

}

DKCommonShapeDrawer::~DKCommonShapeDrawer()
{

}

const DKRect& DKCommonShapeDrawer::Viewport() const
{
    return viewport;
}

void DKCommonShapeDrawer::SetViewport(const DKRect& rc)
{
    viewport = rc;
    this->UpdateTransform();
}

const DKRect& DKCommonShapeDrawer::ContentBounds() const
{
    return contentBounds;
}

void DKCommonShapeDrawer::SetContentBounds(const DKRect& rc)
{
    DKASSERT_DEBUG(rc.size.width > 0.0 && rc.size.height > 0.0);

    this->contentBounds.origin = rc.origin;
    this->contentBounds.size.width = Max(rc.size.width, minimumScaleFactor);
    this->contentBounds.size.height = Max(rc.size.height, minimumScaleFactor);

    this->UpdateTransform();
}

const DKMatrix3& DKCommonShapeDrawer::ContentTransform() const
{
    return contentTM;
}

void DKCommonShapeDrawer::SetContentTransform(const DKMatrix3& tm)
{
    this->contentTM = tm;
    this->UpdateTransform();
}

void DKCommonShapeDrawer::Clear(const DKColor& color) const
{

}

void DKCommonShapeDrawer::RenderText(const DKRect& bounds,
                                     const DKMatrix3& transform,
                                     const DKFoundation::DKString& text,
                                     const DKFont* font,
                                     const DKColor& color,
                                     const DKBlendState& blend) const
{

}

void DKCommonShapeDrawer::RenderText(const DKPoint& baselineBegin,
                                     const DKPoint& baselineEnd,
                                     const DKFoundation::DKString& text,
                                     const DKFont* font,
                                     const DKColor& color,
                                     const DKBlendState& blend) const
{

}

void DKCommonShapeDrawer::UpdateTransform()
{

}

void DKCommonShapeDrawer::DrawPrimitive(DKPrimitiveType p,
                                        const Vertex2D* vertices,
                                        size_t count,
                                        const DKTexture* texture,
                                        const DKSamplerState* sampler,
                                        const DKColor& color,
                                        const DKBlendState& blend) const
{

}

void DKCommonShapeDrawer::DrawPrimitive(DKPrimitiveType p,
                                        const Vertex3D* vertices,
                                        size_t count,
                                        const DKMatrix4& tm,
                                        const DKTexture* texture,
                                        const DKSamplerState* sampler,
                                        const DKBlendState& blend) const
{

}
