//
//  File: DKCanvas.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include "DKCanvas.h"
#include "DKMath.h"
#include "DKAffineTransform2.h"

using namespace DKFramework;

const float DKCanvas::minimumScaleFactor = 0.000001f;

DKCanvas::DKCanvas(DKCommandBuffer* cb, DKTexture* color, DKTexture* depth)
    : commandBuffer(cb)
    , colorAttachment(color)
    , depthAttachment(depth)
    , viewport(0, 0, 1, 1)
    , contentBounds(0, 0, 1, 1)
    , contentTransform(DKMatrix3::identity)
    , screenTransform(DKMatrix3::identity)
{
}

DKCanvas::~DKCanvas()
{
}

const DKRect& DKCanvas::Viewport() const
{
    return viewport;
}

void DKCanvas::SetViewport(const DKRect& rc)
{
    viewport = rc;
    this->UpdateTransform();
}

const DKRect& DKCanvas::ContentBounds() const
{
    return contentBounds;
}

void DKCanvas::SetContentBounds(const DKRect& rc)
{
    DKASSERT_DEBUG(rc.size.width > 0.0 && rc.size.height > 0.0);

    this->contentBounds.origin = rc.origin;
    this->contentBounds.size.width = Max(rc.size.width, minimumScaleFactor);
    this->contentBounds.size.height = Max(rc.size.height, minimumScaleFactor);

    this->UpdateTransform();
}

const DKMatrix3& DKCanvas::ContentTransform() const
{
    return contentTransform;
}

void DKCanvas::SetContentTransform(const DKMatrix3& tm)
{
    this->contentTransform = tm;
    this->UpdateTransform();
}

void DKCanvas::UpdateTransform()
{
    const DKPoint& viewportOffset = this->viewport.origin;
    const DKPoint& contentOffset = this->contentBounds.origin;
    const DKSize& contentScale = this->contentBounds.size;

    DKASSERT_DEBUG(contentScale.width > 0.0 && contentScale.height > 0.0);

    DKMatrix3 targetOrient = this->TargetOrientation();

    DKMatrix3 offset = DKAffineTransform2(-contentOffset.Vector()).Matrix3();
    DKLinearTransform2 s(1.0f / contentScale.width, 1.0f / contentScale.height);
    this->screenTransform = this->contentTransform * offset * DKAffineTransform2(s).Multiply(targetOrient).Matrix3();
}

void DKCanvas::DrawSolidLineStrip(const DKPoint* points,
                                  size_t numPoints,
                                  const DKColor& color)
{
    if (numPoints > 1)
    {
        DKArray<DKPoint> lines;
        lines.Reserve(numPoints * 2);
        for (int i = 0; (i + 1) < numPoints; ++i)
        {
            DKPoint v0 = points[i].Vector().Transform(screenTransform);
            DKPoint v1 = points[i + 1].Vector().Transform(screenTransform);

            lines.Add(v0);
            lines.Add(v1);
        }

        this->DrawSolidLines(lines, lines.Count(), color);
    }
}

void DKCanvas::DrawSolidTriangleStrip(const DKPoint* verts,
                                      size_t numVerts,
                                      const DKColor& color)
{
    if (numVerts > 2)
    {
        DKArray<DKPoint> pts;
        pts.Reserve(numVerts * 3);

        for (size_t i = 0; (i + 2) < numVerts; ++i)
        {
            if (i & 1)
            {
                pts.Add(verts[i + 1]);
                pts.Add(verts[i]);
            }
            else
            {
                pts.Add(verts[i]);
                pts.Add(verts[i + 1]);
            }
            pts.Add(verts[i + 2]);
        }
        this->DrawSolidTriangles(pts, pts.Count(), color);
    }
}

void DKCanvas::DrawTexturedTriangleStrip(const Vertex* verts,
                                         size_t numVerts,
                                         const DKTexture* texture,
                                         const DKColor& color)
{
    if (numVerts > 2 && texture)
    {
        DKArray<Vertex> pts;
        pts.Reserve(numVerts * 3);

        for (size_t i = 0; (i + 2) < numVerts; ++i)
        {
            if (i & 1)
            {
                pts.Add(verts[i + 1]);
                pts.Add(verts[i]);
            }
            else
            {
                pts.Add(verts[i]);
                pts.Add(verts[i + 1]);
            }
            pts.Add(verts[i + 2]);
        }
        this->DrawTexturedTriangles(pts, pts.Count(), texture, color);
    }
}

void DKCanvas::DrawSolidQuad(const SolidQuad& quad, const DKColor& color)
{
    if (IsDrawable())
    {
        const DKVector2 tpos[4] = {
            quad.lt.Vector().Transform(this->contentTransform),			// left-top
            quad.rt.Vector().Transform(this->contentTransform),			// right-top
            quad.lb.Vector().Transform(this->contentTransform),			// left-bottom
            quad.rb.Vector().Transform(this->contentTransform),			// right-bottom
        };
        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const DKPoint vf[6] = {
                quad.lt, quad.lb, quad.rt,
                quad.rt, quad.lb, quad.rb
            };
            DrawSolidTriangles(vf, 6, color);
        }
        else if (t1)
        {
            const DKPoint vf[3] = { quad.lt, quad.lb, quad.rt };
            DrawSolidTriangles(vf, 3, color);
        }
        else if (t2)
        {
            const DKPoint vf[3] = { quad.rt, quad.lb, quad.rb };
            DrawSolidTriangles(vf, 3, color);
        }
    }
}

void DKCanvas::DrawTexturedQuad(const TexturedQuad& quad, const DKTexture* texture, const DKColor& color)
{
    if (IsDrawable() && texture)
    {
        const DKVector2 tpos[4] = {
            quad.lt.position.Vector().Transform(this->contentTransform),			// left-top
            quad.rt.position.Vector().Transform(this->contentTransform),			// right-top
            quad.lb.position.Vector().Transform(this->contentTransform),			// left-bottom
            quad.rb.position.Vector().Transform(this->contentTransform),			// right-bottom
        };
        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const Vertex vf[6] = {
                quad.lt, quad.lb, quad.rt,
                quad.rt, quad.lb, quad.rb
            };
            DrawTexturedTriangles(vf, 6, texture, color);
        }
        else if (t1)
        {
            const Vertex vf[3] = { quad.lt, quad.lb, quad.rt };
            DrawTexturedTriangles(vf, 3, texture, color);
        }
        else if (t2)
        {
            const Vertex vf[3] = { quad.rt, quad.lb, quad.rb };
            DrawTexturedTriangles(vf, 3, texture, color);
        }
    }
}

void DKCanvas::DrawSolidRect(const DKRect& posRect, const DKMatrix3& posTM, const DKColor& color)
{
    if (IsDrawable() && posRect.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(posRect.origin.x, posRect.origin.y).Transform(posTM),                                             // left-top
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y).Transform(posTM),                        // right-top
            DKVector2(posRect.origin.x, posRect.origin.y + posRect.size.height).Transform(posTM),                       // left-bottom
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y + posRect.size.height).Transform(posTM),  // right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const DKPoint vf[6] = {
                pos[0],
                pos[2],
                pos[1],
                pos[1],
                pos[2],
                pos[3],
            };
            DrawSolidTriangles(vf, 6, color);
        }
        else if (t1)
        {
            const DKPoint vf[3] = { pos[0], pos[2],pos[1] };
            DrawSolidTriangles(vf, 3, color);
        }
        else if (t2)
        {
            const DKPoint vf[3] = { pos[1], pos[2], pos[3] };
            DrawSolidTriangles(vf, 3, color);
        }
    }
}

void DKCanvas::DrawTexturedRect(const DKRect& posRect, const DKMatrix3& posTM,
                                const DKRect& texRect, const DKMatrix3& texTM,
                                const DKTexture* texture,
                                const DKColor& color)
{
    if (IsDrawable() && texture && posRect.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(posRect.origin.x, posRect.origin.y).Transform(posTM),                                             // left-top
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y).Transform(posTM),                        // right-top
            DKVector2(posRect.origin.x, posRect.origin.y + posRect.size.height).Transform(posTM),                       // left-bottom
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y + posRect.size.height).Transform(posTM),  // right-bottom
        };
        const DKVector2 tex[4] = {
            DKVector2(texRect.origin.x, texRect.origin.y).Transform(texTM),												// left-top
            DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y).Transform(texTM),                        // right-top
            DKVector2(texRect.origin.x, texRect.origin.y + texRect.size.height).Transform(texTM),						// left-bottom
            DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y + texRect.size.height).Transform(texTM),	// right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const Vertex vf[6] = {
                {pos[0], tex[0]},
                {pos[2], tex[2]},
                {pos[1], tex[1]},
                {pos[1], tex[1]},
                {pos[2], tex[2]},
                {pos[3], tex[3]}
            };
            DrawTexturedTriangles(vf, 6, texture, color);
        }
        else if (t1)
        {
            const Vertex vf[3] = { {pos[0], tex[0]}, {pos[2], tex[2]}, {pos[1], tex[1]} };
            DrawTexturedTriangles(vf, 3, texture, color);
        }
        else if (t2)
        {
            const Vertex vf[3] = { {pos[1], tex[1]}, {pos[2], tex[2]}, {pos[3], tex[3]} };
            DrawTexturedTriangles(vf, 3, texture, color);
        }
    }
}

void DKCanvas::DrawLineEllipse(const DKRect& bounds,
                               const DKMatrix3& tm,
                               const DKColor& color)
{
    if (IsDrawable() && bounds.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 || t2)
        {
            float circleW = Max((tpos[0] - tpos[1]).Length(), (tpos[2] - tpos[3]).Length());
            float circleH = Max((tpos[0] - tpos[2]).Length(), (tpos[1] - tpos[3]).Length());
            const int numSegments = LineSegmentsCircumference(Min(circleW, circleH) * 0.5f);

            DKArray<DKPoint> lines;
            lines.Resize(numSegments + 1);

            const DKVector2 center = bounds.Center().Vector();
            const DKVector2 radius = bounds.size.Vector() * 0.5f;
            const DKVector2 radiusSq = { radius.x * radius.x, radius.y * radius.y };

            // formula: X^2 / A^2 + Y^2 / B^2 = 1
            // A = bounds.width/2, B = bounds.height/2
            // generate outline polygon!
            for (int i = 0; i < numSegments; ++i)
            {
                // x = a*sin(t)
                // y = b*cos(t)
                // where 0 <= t < 2*PI
                double t = (DKGL_PI * 2.0) * (float(i) / float(numSegments));
                DKVector2 p = { radius.x * sinf(t), radius.y * cosf(t) };

                lines[i] = DKVector2(p + center).Transform(tm);
            }
            lines[numSegments] = lines.Value(0);
            DrawSolidLines(lines, lines.Count(), color);
        }
    }
}

void DKCanvas::DrawSolidEllipse(const DKRect& bounds, const DKMatrix3& tm, const DKColor& color)
{
    if (IsDrawable() && bounds.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 || t2)
        {
            float circleW = Max((tpos[0] - tpos[1]).Length(), (tpos[2] - tpos[3]).Length());
            float circleH = Max((tpos[0] - tpos[2]).Length(), (tpos[1] - tpos[3]).Length());
            const int numSegments = LineSegmentsCircumference(Min(circleW, circleH) * 0.5f);

            DKArray<DKPoint> triangleVertices;
            triangleVertices.Resize(numSegments * 3);

            const DKVector2 center = bounds.Center().Vector();
            const DKVector2 radius = bounds.size.Vector() * 0.5f;
            const DKVector2 radiusSq = { radius.x * radius.x, radius.y * radius.y };

            DKVector2 lastVertex = DKVector2(center.x, center.y + radius.y).Transform(tm);
            DKVector2 transCenter = DKVector2(center).Transform(tm);
            // formula: X^2 / A^2 + Y^2 / B^2 = 1
            // A = bounds.width/2, B = bounds.height/2
            // generate outline polygon!
            for (int i = 0; (i + 1) < numSegments; ++i)
            {
                // x = a*sin(t)
                // y = b*cos(t)
                // where 0 <= t < 2*PI
                double t = (DKGL_PI * 2.0) * (float(i) / float(numSegments));
                DKVector2 p = { radius.x * sinf(t), radius.y * cosf(t) };
                DKVector2 vertex = DKVector2(p + center).Transform(tm);

                triangleVertices[i * 3] = vertex;
                triangleVertices[i * 3 + 1] = lastVertex;
                triangleVertices[i * 3 + 2] = transCenter;
                lastVertex = vertex;
            }
            triangleVertices[numSegments * 3 - 3] = triangleVertices[1];
            triangleVertices[numSegments * 3 - 2] = lastVertex;
            triangleVertices[numSegments * 3 - 1] = transCenter;

            DrawSolidTriangles(triangleVertices, triangleVertices.Count(), color);
        }
    }
}

void DKCanvas::DrawTexturedEllipse(const DKRect& bounds, const DKMatrix3& tm,
                                   const DKRect& texBounds, const DKMatrix3& texTm,
                                   const DKTexture* texture, const DKColor& color)
{
    if (IsDrawable() && texture && bounds.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 || t2)
        {
            float circleW = Max((tpos[0] - tpos[1]).Length(), (tpos[2] - tpos[3]).Length());
            float circleH = Max((tpos[0] - tpos[2]).Length(), (tpos[1] - tpos[3]).Length());
            const int numSegments = LineSegmentsCircumference(Min(circleW, circleH) * 0.5f);

            DKArray<Vertex> triangleVertices;
            triangleVertices.Resize(numSegments * 3);

            const DKVector2 center = bounds.Center().Vector();
            const DKVector2 texCenter = texBounds.Center().Vector();
            const DKVector2 radius = bounds.size.Vector() * 0.5f;
            const DKVector2 radiusSq = { radius.x * radius.x, radius.y * radius.y };

            Vertex lastVertex = {
                DKVector2(center.x, center.y + radius.y).Transform(tm),
                DKVector2(texCenter.x, texCenter.y + texBounds.size.height * (radius.y / bounds.size.height)).Transform(texTm)
            };
            Vertex transCenter = {
                DKVector2(center).Transform(tm),
                DKVector2(texCenter).Transform(texTm)
            };

            // formula: X^2 / A^2 + Y^2 / B^2 = 1
            // A = bounds.width/2, B = bounds.height/2
            // generate outline polygon!
            for (int i = 0; (i + 1) < numSegments; ++i)
            {
                // x = a*sin(t)
                // y = b*cos(t)
                // where 0 <= t < 2*PI
                double t = (DKGL_PI * 2.0) * (float(i) / float(numSegments));
                DKVector2 p = DKVector2(radius.x * sin(t), radius.y * cos(t)) + center;
                DKVector2 uv = texBounds.origin.Vector() + texBounds.size.Vector() * ((p - bounds.origin.Vector()) / bounds.size.Vector());
                Vertex vertex = {
                    DKVector2(p).Transform(tm),
                    DKVector2(uv).Transform(texTm)
                };

                triangleVertices[i * 3] = vertex;
                triangleVertices[i * 3 + 1] = lastVertex;
                triangleVertices[i * 3 + 2] = transCenter;
                lastVertex = vertex;
            }
            triangleVertices[numSegments * 3 - 3] = triangleVertices[1];
            triangleVertices[numSegments * 3 - 2] = lastVertex;
            triangleVertices[numSegments * 3 - 1] = transCenter;

            DrawTexturedTriangles(triangleVertices, triangleVertices.Count(), texture, color);
        }
    }
}

void DKCanvas::DrawText(const DKRect& bounds,
                        const DKMatrix3& transform,
                        const DKString& text,
                        const DKFont* font,
                        const DKColor& color)
{
    if (!IsDrawable() || !bounds.IsValid())
        return;
    size_t textLen = text.Length();
    if (font == nullptr || !font->IsValid() || textLen == 0)
        return;

    struct Quad
    {
        TexturedQuad quad;
        const DKTexture* texture;
    };
    auto orderByTextureAsc = [](const Quad& lhs, const Quad& rhs)
    {
        return reinterpret_cast<uintptr_t>(lhs.texture) > reinterpret_cast<uintptr_t>(rhs.texture);
    };
    DKArray<Quad> quads;
    quads.Reserve(textLen);

    DKPoint bboxMin(0, 0);
    DKPoint bboxMax(0, 0);
    float offset = 0;       // accumulated text width (pixel)

    for (size_t i = 0; i < textLen; ++i)
    {
        // get glyph info from font object
        const DKFont::GlyphData* glyph = font->GlyphDataForChar(text[i]);
        if (glyph == nullptr)
            continue;

        DKPoint posMin(offset + glyph->position.x, -glyph->position.y);
        DKPoint posMax(offset + glyph->position.x + glyph->frame.size.width, glyph->frame.size.height - glyph->position.y);

        if (bboxMin.x > posMin.x) bboxMin.x = posMin.x;
        if (bboxMin.y > posMin.y) bboxMin.y = posMin.y;
        if (bboxMax.x < posMax.x) bboxMax.x = posMax.x;
        if (bboxMax.y < posMax.y) bboxMax.y = posMax.y;

        if (glyph->texture)
        {
            uint32_t textureWidth = glyph->texture->Width();
            uint32_t textureHeight = glyph->texture->Height();
            if (textureWidth > 0 && textureHeight > 0)
            {
                float invW = 1.0f / static_cast<float>(textureWidth);
                float invH = 1.0f / static_cast<float>(textureHeight);

                DKPoint uvMin(glyph->frame.origin.x * invW, glyph->frame.origin.y * invH);
                DKPoint uvMax((glyph->frame.origin.x + glyph->frame.size.width) * invW,
                              (glyph->frame.origin.y + glyph->frame.size.height) * invH);

                const Quad q =
                {
                    Vertex { DKVector2(posMin.x, posMin.y), DKVector2(uvMin.x, uvMin.y)}, // lt
                    Vertex { DKVector2(posMax.x, posMin.y), DKVector2(uvMax.x, uvMin.y)}, // rt
                    Vertex { DKVector2(posMin.x, posMax.y), DKVector2(uvMin.x, uvMax.y)}, // lb
                    Vertex { DKVector2(posMax.x, posMax.y), DKVector2(uvMax.x, uvMax.y)}, // rb
                    glyph->texture,
                };
                quads.Add(q);
            }
        }
        offset += glyph->advance.width + font->KernAdvance(text[i], text[i + 1]).x; // text[i+1] can be null.
    }
    if (quads.Count() == 0)
        return;

    const float width = bboxMax.x - bboxMin.x;
    const float height = bboxMax.y - bboxMin.y;

    if (width <= 0.0f || height <= 0.0f)
        return;

    // sort by texture order
    quads.Sort([](const Quad& lhs, const Quad& rhs)
    {
        return reinterpret_cast<uintptr_t>(lhs.texture) > reinterpret_cast<uintptr_t>(rhs.texture);
    });

    // calculate transform matrix
    DKAffineTransform2 trans;
    trans.Translate(-bboxMin.x, -bboxMin.y);    // move origin
    trans *= DKLinearTransform2().Scale(1.0f / width, 1.0f / height); // normalize size
    trans *= DKLinearTransform2().Scale(bounds.size.width, bounds.size.height); // scale to bounds
    trans.Translate(bounds.origin.x, bounds.origin.y); // move to bounds origin

    DKMatrix3 matrix = trans.Matrix3();
    matrix *= transform; // user transform

    const DKTexture* lastTexture = nullptr;
    DKArray<Vertex> triangles;
    triangles.Reserve(quads.Count() * 6);
    for (Quad& q : quads)
    {
        if (q.texture != lastTexture)
        {
            if (triangles.Count() > 0)
                DrawTexturedTriangles(triangles, triangles.Count(), lastTexture, color);
            triangles.Clear();
            lastTexture = q.texture;
        }
        Vertex vf[6] = {
            Vertex { q.quad.lt.position.Vector().Transform(matrix), q.quad.lt.texcoord },
            Vertex { q.quad.lb.position.Vector().Transform(matrix), q.quad.lb.texcoord },
            Vertex { q.quad.rt.position.Vector().Transform(matrix), q.quad.rt.texcoord },
            Vertex { q.quad.rt.position.Vector().Transform(matrix), q.quad.rt.texcoord },
            Vertex { q.quad.lb.position.Vector().Transform(matrix), q.quad.lb.texcoord },
            Vertex { q.quad.rb.position.Vector().Transform(matrix), q.quad.rb.texcoord },
        };
        triangles.Add(vf, 6);
    }
    if (triangles.Count() > 0)
        DrawTexturedTriangles(triangles, triangles.Count(), lastTexture, color);
}

void DKCanvas::DrawText(const DKPoint& baselineBegin,
                        const DKPoint& baselineEnd,
                        const DKString& text,
                        const DKFont* font,
                        const DKColor& color)
{
    if (IsDrawable() == false)
        return;
    if (font == nullptr || !font->IsValid() || text.Length() == 0)
        return;

    if ((baselineEnd.Vector() - baselineBegin.Vector()).Length() < FLT_EPSILON)
        return;

    // font size, screen size in pixel units
    const float lineHeight = font->LineHeight();
    const float lineWidth = font->LineWidth(text);
    const DKRect textBounds = font->Bounds(text);

    const DKSize& viewportSize = this->viewport.size;
    const DKSize& contentScale = this->contentBounds.size;

    // change local-coords to pixel-coords
    const DKSize scaleToScreen = DKSize(viewportSize.width / contentScale.width, viewportSize.height / contentScale.height);
    const DKVector2 baselinePixelBegin = DKVector2(baselineBegin.x * scaleToScreen.width, baselineBegin.y * scaleToScreen.height);
    const DKVector2 baselinePixelEnd = DKVector2(baselineEnd.x * scaleToScreen.width, baselineEnd.y * scaleToScreen.height);
    const float scale = (baselinePixelEnd - baselinePixelBegin).Length();
    const DKVector2 baselinePixelDir = (baselinePixelEnd - baselinePixelBegin).Normalize();
    const float angle = acosf(baselinePixelDir.x) * ((baselinePixelDir.y < 0) ? -1.0f : 1.0f);

    // calculate transform (matrix)
    DKAffineTransform2 transform(
        DKLinearTransform2()
        .Scale(scale / lineWidth)										// scale
        .Rotate(angle)													// rotate
        .Scale(1.0f / viewportSize.width, 1.0f / viewportSize.height)		// normalize (0~1)
        .Scale(contentScale.width, contentScale.height)					// apply contentScale
        , baselineBegin.Vector());

    DrawText(textBounds, transform.Matrix3(), text, font, color);
}

uint32_t DKCanvas::LineSegmentsCircumference(float radius) const
{
    float circleAreaPoints = radius * radius * static_cast<float>(DKGL_PI);
    float contentArea = contentBounds.size.width * contentBounds.size.height;
    float resolutionArea = viewport.size.width * viewport.size.height;
    float areaRatio = resolutionArea / contentArea;
    float circleAreaPixels = circleAreaPoints * areaRatio;
    float r2 = sqrtf(circleAreaPixels * static_cast<float>(DKGL_MATH_1_PI));
    float circumference = r2 * static_cast<float>(DKGL_PI) * 2.0f;

    constexpr uint32_t minSegments = 4;
    constexpr uint32_t maxSegments = 90;
    uint32_t numSegments = Clamp(static_cast<uint32_t>(circumference * 0.25f),
                                     minSegments,
                                     maxSegments);

    return numSegments;
}

