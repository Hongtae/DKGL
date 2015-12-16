//
//  File: DKFrame.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKFrame.h"
#include "DKScreen.h"
#include "DKVector2.h"
#include "DKAffineTransform2.h"

using namespace DKFoundation;
using namespace DKFramework;

DKFrame::DKFrame(void)
	: transform(DKMatrix3::identity)
	, transformInverse(DKMatrix3::identity)
	, superframe(NULL)
	, screen(NULL)
	, loaded(false)
	, enableVisibilityTest(true)
	, drawSurface(false)
	, contentResolution(DKSize(1, 1))
	, contentScale(1, 1)
	, contentTransform(DKMatrix3::identity)
	, contentTransformInverse(DKMatrix3::identity)
	, color(DKColor(1, 1, 1, 1).RGBA32Value())
	, blendState(DKBlendState::defaultOpaque)
	, hidden(false)
	, enabled(true)
	, depthFormat(DKRenderTarget::DepthFormat24)
{
}

DKFrame::~DKFrame(void)
{
	// a frame must be unloaded before beging destroyed.
	DKASSERT_DESC_DEBUG(IsLoaded() == false, "Frame must unload before beging destroyed!");

	RemoveFromSuperframe();

	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		subFramesCopy.Value(index)->RemoveFromSuperframe();
	}
}

DKScreen* DKFrame::Screen(void)
{
	return screen;
}

const DKScreen* DKFrame::Screen(void) const
{
	return screen;
}

bool DKFrame::AddSubframe(DKFrame* frame)
{
	if (frame == NULL)
		return false;

	if (frame->superframe == NULL)
	{
		//this->subframes.Add(frame);
		this->subframes.Insert(frame, 0);	// bring to front
		frame->superframe = this;
		if (this->loaded)
		{
			frame->Load(this->screen, this->ContentResolution());
			frame->UpdateContentResolution();
			SetRedraw();
		}
		return true;
	}
	return false;
}

void DKFrame::RemoveSubframe(DKFrame* frame)
{
	if (frame == NULL)
		return;

	if (frame && frame->superframe == this)
	{
		frame->ReleaseMouseData();
		for (FrameArray::Index index = 0; index < subframes.Count(); index++)
		{
			if (subframes.Value(index) == frame)
			{
				frame->superframe = NULL;
				subframes.Remove(index); // object can be destroyed now
				SetRedraw();
				return;
			}
		}
	}
}

void DKFrame::RemoveFromSuperframe(void)
{
	if (superframe)
		superframe->RemoveSubframe(this);
}

bool DKFrame::BringSubframeToFront(DKFrame* frame)
{
	if (frame && frame->superframe == this)
	{
		for (FrameArray::Index index = 0; index < this->subframes.Count(); ++index)
		{
			if (this->subframes.Value(index) == frame)
			{
				if (index > 0)
				{
					// temporary hold an ownership to prevent object destruction.
					DKObject<DKFrame> tmp(frame);
					this->subframes.Remove(index);
					this->subframes.Insert(frame, 0);
					this->SetRedraw();
				}
				return true;
			}
		}
	}
	return false;
}

bool DKFrame::SendSubframeToBack(DKFrame* frame)
{
	if (frame && frame->superframe == this)
	{
		frame->ReleaseMouseData();
		for (FrameArray::Index index = 0; index < this->subframes.Count(); ++index)
		{
			if (this->subframes.Value(index) == frame)
			{
				if (index + 1 < this->subframes.Count())
				{
					// temporary hold an ownership to prevent object destruction.
					DKObject<DKFrame> tmp(frame);
					this->subframes.Remove(index);
					this->subframes.Add(frame);
					this->SetRedraw();
				}
				return true;
			}
		}
	}
	return false;
}

DKFrame::FrameArray DKFrame::Subframes(void) const
{
	return subframes;
}

DKFrame* DKFrame::Superframe(void)
{
	return superframe;
}

const DKFrame* DKFrame::Superframe(void) const
{
	return superframe;
}

DKFrame* DKFrame::SubframeAtIndex(unsigned int index)
{
	if (index < subframes.Count())
		return subframes.Value(index);
	return NULL;
}

const DKFrame* DKFrame::SubframeAtIndex(unsigned int index) const
{
	if (index < subframes.Count())
		return subframes.Value(index);
	return NULL;
}

bool DKFrame::IsDescendantOf(const DKFrame* frame) const
{
	if (frame == NULL)
		return false;
	if (frame == this)
		return true;
	if (superframe == NULL)
		return false;

	return superframe->IsDescendantOf(frame);
}

size_t DKFrame::NumberOfSubframes(void) const
{
	return subframes.Count();
}

size_t DKFrame::NumberOfDescendants(void) const
{
	size_t num = 1;
	for (size_t i = 0; i < subframes.Count(); ++i)
		num += subframes.Value(i)->NumberOfDescendants();
	return num;
}

DKFrame* DKFrame::FrameAtPosition(const DKPoint& pos, FrameFilter* filter)
{
	return const_cast<DKFrame*>(static_cast<const DKFrame*>(this)->FrameAtPosition(pos, filter));
}

const DKFrame* DKFrame::FrameAtPosition(const DKPoint& pos, FrameFilter* filter) const
{
	if (DKRect(0, 0, 1, 1).IsPointInside(pos))
	{
		DKVector2 localPos = DKVector2(pos.x * this->contentScale.width, pos.y * this->contentScale.height);
		localPos.Transform(this->contentTransformInverse);

		if (filter && !filter->Invoke(this))
			return NULL;

		if (!this->HitTest(localPos))
			return NULL;

		if (this->ContentHitTest(localPos))
		{
			for (FrameArray::Index index = 0; index < subframes.Count(); index++)
			{
				const DKFrame* frame = subframes.Value(index);
				if (frame->IsHidden())
					continue;

				DKMatrix3 tm = frame->TransformInverse();
				DKVector2 posInFrame = DKVector2(localPos).Transform(tm);

				const DKFrame* target = frame->FrameAtPosition(posInFrame, filter);
				if (target)
					return target;
			}
		}
	}
	return NULL;
}

void DKFrame::Load(DKScreen* screen, const DKSize& resolution)
{
	if (this->screen != screen)
	{
		Unload();
		this->screen = screen;
		if (this->screen)
		{
			DKSize res = resolution;
			if (res.width < 1.0f)
				res.width = 1.0f;
			if (res.height < 1.0f)
				res.height = 1.0f;

			this->screen->RegisterAutoUnloadFrame(this);
			this->contentResolution = res;
			this->contentResolution = this->DKFrame::QueryContentResolution();
			this->OnLoaded();
			this->loaded = true;
			this->OnContentResized();
			this->UpdateContentResolution();

			FrameArray subFramesCopy = Subframes();
			for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
			{
				subFramesCopy.Value(index)->Load(screen, resolution);
			}

			SetRedraw();
		}
	}
}

void DKFrame::Unload(void)
{
	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		subFramesCopy.Value(index)->Unload();
	}

	if (this->loaded)
	{
		this->ReleaseMouseData();
		this->OnUnload();
		this->renderer = NULL;
		this->screen->UnregisterAutoUnloadFrame(this);
		this->screen->RemoveKeyFrameForAnyDevices(this, false);
		this->screen->RemoveFocusFrameForAnyDevices(this, false);
	}
	//this->contentResolution = DKSize(1,1);
	this->screen = NULL;
	this->loaded = false;
}

bool DKFrame::IsLoaded(void) const
{
	return loaded;
}

void DKFrame::SetTransform(const DKMatrix3& transform)
{
	if (screen && screen->RootFrame() == this)
	{
		DKLog("RootFrame's transform cannot be changed.\n");
	}
	else
	{
		if (this->transform != transform)
		{
			this->transform = transform;
			this->transformInverse = DKMatrix3(transform).Inverse();
			//this->ReleaseMouseData();
			this->UpdateContentResolution();

			if (this->superframe)
				superframe->SetRedraw();
		}
	}
}

const DKMatrix3& DKFrame::Transform(void) const
{
	return transform;
}

const DKMatrix3& DKFrame::TransformInverse(void) const
{
	return transformInverse;
}

DKMatrix3 DKFrame::LocalFromRootTransform(void) const
{
	DKMatrix3 tm = superframe ? superframe->LocalFromRootTransform() : DKMatrix3::identity;
	return tm * this->LocalFromSuperTransform();
}

DKMatrix3 DKFrame::LocalToRootTransform(void) const
{
	DKMatrix3 tm = superframe ? superframe->LocalToRootTransform() : DKMatrix3::identity;
	return this->LocalToSuperTransform() * tm;
}

DKMatrix3 DKFrame::LocalFromSuperTransform(void) const
{
	DKMatrix3 tm = DKMatrix3::identity;
	if (superframe)
	{
		// to normalized local coordinates.
		tm.Multiply(this->transformInverse);

		// apply local content scale.
		tm.Multiply(DKAffineTransform2(DKLinearTransform2().Scale(contentScale.width, contentScale.height)).Matrix3());

		// apply inversed content transform.
		tm.Multiply(this->contentTransformInverse);
	}
	return tm;
}

DKMatrix3 DKFrame::LocalToSuperTransform(void) const
{
	DKMatrix3 tm = DKMatrix3::identity;
	if (superframe)
	{
		// apply content transform.
		tm.Multiply(this->contentTransform);

		// normalize local scale to (0.0 ~ 1.0)
		tm.Multiply(DKAffineTransform2(DKLinearTransform2().Scale(1.0f / contentScale.width, 1.0f / contentScale.height)).Matrix3());

		// transform to parent
		tm.Multiply(this->transform);
	}
	return tm;
}

DKSize DKFrame::QueryContentResolution(void) const
{
	if (superframe)
	{
		if (superframe->ContentResolution().width > 0 && superframe->ContentResolution().height > 0)
		{
			// get each points of box (not rect)
			float w = contentScale.width;
			float h = contentScale.height;

			DKPoint lb = superframe->LocalToPixel(this->LocalToSuper(DKPoint(0,0)));		// left-bottom
			DKPoint lt = superframe->LocalToPixel(this->LocalToSuper(DKPoint(0,h)));		// left-top
			DKPoint rt = superframe->LocalToPixel(this->LocalToSuper(DKPoint(w,h)));		// right-top
			DKPoint rb = superframe->LocalToPixel(this->LocalToSuper(DKPoint(w,0)));		// right-bottom

			DKVector2 horizontal1 = rb.Vector() - lb.Vector();		// vertical length 1
			DKVector2 horizontal2 = rt.Vector() - lt.Vector();		// vertical length 2
			DKVector2 vertical1 = lt.Vector() - lb.Vector();		// horizontal length 1
			DKVector2 vertical2 = rt.Vector() - rb.Vector();		// horizontal length 2

			DKSize result = DKSize(Max<float>(horizontal1.Length(), horizontal2.Length()), Max<float>(vertical1.Length(), vertical2.Length()));
			// round to be int
			result.width = floor(result.width + 0.5f);
			result.height = floor(result.height + 0.5f);
			return result;
		}
	}
	return ContentResolution();
}

void DKFrame::UpdateContentResolution()
{
	if (!this->loaded)
		return;

	bool resized = false;
	if (screen && screen->RootFrame() == this)
	{
		DKSize size = screen->ScreenResolution();
		unsigned int width = Max<int>(floor(size.width + 0.5f), 1);
		unsigned int height = Max<int>(floor(size.height + 0.5f), 1);
		if ((int)floor(contentResolution.width + 0.5f) != width || (int)floor(contentResolution.height + 0.5f) != height)
		{
			//DKLog("DKFrame (0x%x, root) resized. (%dx%d -> %dx%d)\n", this, (int)contentResolution.width, (int)contentResolution.height, width, height);
			resized = true;
			contentResolution.width = width;
			contentResolution.height = height;			
		}
	}
	else
	{
		DKSize size = QueryContentResolution();

		int maxTexSize = DKTexture::MaxTextureSize();
		int width = Clamp<int>(floor(size.width + 0.5f), 1, maxTexSize);
		int height = Clamp<int>(floor(size.height + 0.5f), 1, maxTexSize);

		if ((int)floor(contentResolution.width + 0.5f) != width || (int)floor(contentResolution.height + 0.5f) != height)
		{
			//DKLog("DKFrame (0x%x) resized. (%dx%d -> %dx%d)\n", this, (int)contentResolution.width, (int)contentResolution.height, width, height);
			resized = true;
			contentResolution.width = width;
			contentResolution.height = height;
			renderer = NULL; // created on render.
		}
	}

	DKASSERT_DEBUG(this->contentResolution.width > 0.0f && this->contentResolution.height > 0.0f);

	if (resized)
	{
		OnContentResized();		
		SetRedraw();
	}

	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		subFramesCopy.Value(index)->UpdateContentResolution();
	}
}

const DKSize& DKFrame::ContentResolution(void) const
{
	return contentResolution;
}

bool DKFrame::CaptureKeyboard(int deviceId)
{
	if (screen == NULL)
		return false;
	if (this->CanHandleKeyboard() == false)
		return false;

	return screen->SetKeyFrame(deviceId, this);
}

bool DKFrame::CaptureMouse(int deviceId)
{
	if (screen == NULL)
		return false;
	if (this->CanHandleMouse() == false)
		return false;

	return screen->SetFocusFrame(deviceId, this);
}

void DKFrame::ReleaseKeyboard(int deviceId)
{
	if (screen)
		screen->RemoveKeyFrame(deviceId, this);
}

void DKFrame::ReleaseMouse(int deviceId)
{
	if (screen)
		screen->RemoveFocusFrame(deviceId, this);
}

void DKFrame::ReleaseAllKeyboardsCapturedBySelf(void)
{
	if (screen)
		screen->RemoveKeyFrameForAnyDevices(this, false);
}

void DKFrame::ReleaseAllMiceCapturedBySelf(void)
{
	if (screen)
		screen->RemoveFocusFrameForAnyDevices(this, false);
}

bool DKFrame::IsKeybaordCapturedBySelf(int deviceId) const
{
	if (screen)
	{
		if (this->IsDescendantOf(screen->RootFrame()))
			return screen->KeyFrame(deviceId) == this;
	}
	return false;
}

bool DKFrame::IsMouseCapturedBySelf(int deviceId) const
{
	if (screen)
	{
		if (this->IsDescendantOf(screen->RootFrame()))
			return screen->FocusFrame(deviceId) == this;
	}
	return false;
}

DKPoint DKFrame::MousePosition(int deviceId) const
{
	if (screen && screen->Window())
	{
		const DKWindow* window = screen->Window();
		const DKFrame* rootFrame = screen->RootFrame();
		if (IsDescendantOf(rootFrame))
		{
			// transform mouse pos to screen coordinates (0.0 ~ 1.0)
			DKVector2 pos = screen->WindowToScreen(window->MousePosition(deviceId)).Vector();

			// convert to root-frame's coordinates
			const DKSize& scale = rootFrame->contentScale;
			DKMatrix3 tm = DKMatrix3::identity;
			tm.Multiply(DKAffineTransform2(DKLinearTransform2(scale.width, scale.height)).Matrix3());
			tm.Multiply(rootFrame->contentTransformInverse);

			// convert to local (this) coordinates.
			tm.Multiply(this->LocalFromRootTransform());
			return pos.Transform(tm);
		}
	}
	return DKPoint(-1,-1);
}

bool DKFrame::IsMouseHover(int deviceId) const
{
	bool hover = false;
	const DKMap<int, bool>::Pair* p = mouseHover.Find(deviceId);
	if (p)
		hover = p->value;
	return hover;
}

const DKSize& DKFrame::ContentScale(void) const
{
	return contentScale;
}

void DKFrame::SetContentScale(float w, float h)
{
	DKASSERT_DEBUG(w > 0.0f && h > 0.0f);
	
	w = Max(w, DKRenderer::minimumScaleFactor);
	h = Max(h, DKRenderer::minimumScaleFactor);
	
	if (w == contentScale.width && h == contentScale.height)
	{
	}
	else
	{
		contentScale = DKSize(w,h);
		SetRedraw();
	}
}

void DKFrame::SetContentScale(const DKSize& s)
{
	SetContentScale(s.width, s.height);
}

DKRect DKFrame::Bounds(void) const
{
	return DKRect(0,0,contentScale.width,contentScale.height);
}

DKRect DKFrame::DisplayBounds(void) const
{
	DKRect rc = this->Bounds();
	DKVector2 v0(rc.origin.x, rc.origin.y);
	DKVector2 v1(rc.origin.x, rc.origin.y + rc.size.height);
	DKVector2 v2(rc.origin.x + rc.size.width, rc.origin.y);
	DKVector2 v3(rc.origin.x + rc.size.width, rc.origin.y + rc.size.height);

	v0.Transform(this->contentTransformInverse);
	v1.Transform(this->contentTransformInverse);
	v2.Transform(this->contentTransformInverse);
	v3.Transform(this->contentTransformInverse);

	DKVector2 minp = v0;
	DKVector2 maxp = v0;

	for (const DKVector2& v : {v1, v2, v3})
	{
		if (minp.x > v.x)		minp.x = v.x;
		if (minp.y > v.y)		minp.y = v.y;
		if (maxp.x < v.x)		maxp.x = v.x;
		if (maxp.y < v.y)		maxp.y = v.y;
	}
	return DKRect(minp, maxp - minp);
}

void DKFrame::SetContentTransform(const DKMatrix3& m)
{
	if (this->contentTransform != m)
	{
		DKMatrix3 inv = DKMatrix3::identity;
		if (m.GetInverseMatrix(inv, NULL))
		{
			this->contentTransform = m;
			this->contentTransformInverse = inv;
		}
		else
		{
			this->contentTransform = DKMatrix3::identity;
			this->contentTransformInverse = DKMatrix3::identity;
		}
		this->SetRedraw();
	}
}

const DKMatrix3& DKFrame::ContentTransform(void) const
{
	return this->contentTransform;
}

const DKMatrix3& DKFrame::ContentTransformInverse(void) const
{
	return this->contentTransformInverse;
}

DKPoint DKFrame::LocalToSuper(const DKPoint& pt) const
{
	if (superframe)
	{
		DKASSERT_DEBUG(contentScale.width > 0.0f && contentScale.height > 0.0f);

		DKVector2 v = pt.Vector();

		// apply content transform.
		v.Transform(this->contentTransform);

		// normalize coordinates (0.0 ~ 1.0)
		v.x = v.x / this->contentScale.width;
		v.y = v.y / this->contentScale.height;

		// transform to parent.
		v.Transform(this->transform);

		return v;
	}
	return pt;
}

DKPoint DKFrame::SuperToLocal(const DKPoint& pt) const
{
	if (superframe)
	{
		DKVector2 v = pt.Vector();

		// apply inversed transform to normalize coordinates (0.0 ~ 1.0)
		v.Transform(this->transformInverse);

		// apply content scale
		v.x = v.x * this->contentScale.width;
		v.y = v.y * this->contentScale.height;

		// apply inversed content transform
		v.Transform(this->contentTransformInverse);

		return v;
	}
	return pt;
}

DKPoint DKFrame::LocalToPixel(const DKPoint& pt) const
{
	DKASSERT_DEBUG(contentScale.width > 0.0f && contentScale.height > 0.0f);

	DKVector2 v = pt.Vector();

	// apply content transform.
	v.Transform(this->contentTransform);

	// normalize coordinates (0.0 ~ 1.0)
	v.x = v.x / this->contentScale.width;
	v.y = v.y / this->contentScale.height;

	// convert to pixel-space.
	v.x = v.x * this->contentResolution.width;
	v.y = v.y * this->contentResolution.height;

	return v;
}

DKPoint DKFrame::PixelToLocal(const DKPoint& pt) const
{
	DKASSERT_DEBUG(this->contentResolution.width > 0.0f && this->contentResolution.height > 0.0f);

	DKVector2 v = pt.Vector();

	// normalize coordinates.
	v.x = v.x / this->contentResolution.width;
	v.y = v.y / this->contentResolution.height;

	// apply content scale.
	v.x = v.x * this->contentScale.width;
	v.y = v.y * this->contentScale.height;

	// apply inversed content transform.
	v.Transform(this->contentTransformInverse);

	return v;
}

DKSize DKFrame::LocalToPixel(const DKSize& size) const
{
	DKPoint p0 = LocalToPixel(DKPoint(0, 0));
	DKPoint p1 = LocalToPixel(DKPoint(size.width, size.height));

	return DKSize(p1.x - p0.x, p1.y - p0.y);
}

DKSize DKFrame::PixelToLocal(const DKSize& size) const
{
	DKPoint p0 = PixelToLocal(DKPoint(0, 0));
	DKPoint p1 = PixelToLocal(DKPoint(size.width, size.height));

	return DKSize(p1.x - p0.x, p1.y - p0.y);
}

DKRect DKFrame::LocalToPixel(const DKRect& rect) const
{
	return DKRect(LocalToPixel(rect.origin), LocalToPixel(rect.size));
}

DKRect DKFrame::PixelToLocal(const DKRect& rect) const
{
	return DKRect(PixelToLocal(rect.origin), PixelToLocal(rect.size));
}

const DKTexture2D* DKFrame::Texture(void) const
{
	if (renderer)
		return renderer->RenderTarget()->ColorTexture(0);
	return NULL;
}

void DKFrame::SetRedraw(void) const
{
	drawSurface = true;
}

void DKFrame::Render(void) const
{
	const_cast<DKFrame*>(this)->RenderInternal();
}

bool DKFrame::InsideFrameRect(bool* covered, const DKRect& rect, const DKMatrix3& tm) const
{
	DKVector2 outerPos[4] = {
		DKVector2(rect.origin.x, rect.origin.y),										// left-bottom
		DKVector2(rect.origin.x, rect.origin.y + rect.size.height),						// left-top
		DKVector2(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height),	// right-top
		DKVector2(rect.origin.x + rect.size.width, rect.origin.y)						// right-bottom
	};

	// apply parent space transform.
	DKMatrix3 m = tm * this->transformInverse;
	for (DKVector2& v : outerPos)
		v.Transform(m);
	
	// check outPos is inside rect.
	if (outerPos[0].x >= 0.0 && outerPos[0].x <= 1.0 && outerPos[0].y >= 0.0 && outerPos[0].y <= 1.0 &&
		outerPos[1].x >= 0.0 && outerPos[1].x <= 1.0 && outerPos[1].y >= 0.0 && outerPos[1].y <= 1.0 &&
		outerPos[2].x >= 0.0 && outerPos[2].x <= 1.0 && outerPos[2].y >= 0.0 && outerPos[2].y <= 1.0 &&
		outerPos[3].x >= 0.0 && outerPos[3].x <= 1.0 && outerPos[3].y >= 0.0 && outerPos[3].y <= 1.0)
	{
		// frame rect covered parent frame entirely.
		if (covered)
			*covered = true;
		return true;
	}

	if (covered)
		*covered = false;

	// check normalized local rect(0,0,1,1) is inside parent rect.
	return rect.IntersectRect(DKRect(0,0,1,1), this->transform);
}

bool DKFrame::RenderInternal(void)
{
	DKASSERT_DESC_DEBUG(IsLoaded(), "Frame must be initialized with screen!");
	DKASSERT_DEBUG(this->contentResolution.width > 0.0f && this->contentResolution.height > 0.0f);

	if (renderer == NULL)
	{
		int width = floor(this->contentResolution.width + 0.5f);
		int height = floor(this->contentResolution.height + 0.5f);
		DKObject<DKRenderTarget> renderTarget = DKRenderTarget::Create(width, height, this->depthFormat); 
		this->renderer = DKObject<DKRenderer>::New(renderTarget);
		//DKLog("Create renderer (%dx%d) for DKFrame:0x%x\n", width, height, this);
		this->drawSurface = true;
	}
	renderer->SetViewport(DKRect(0,0,contentResolution.width,contentResolution.height));
	renderer->SetContentBounds(DKRect(0, 0, this->contentScale.width, this->contentScale.height));
	renderer->SetContentTransform(this->contentTransform);

	bool drawSelf = false;
	const DKRect bounds = this->Bounds();
	for (FrameArray::Index index = 0; index < subframes.Count(); index++)
	{
		DKFrame* frame = subframes.Value(index);
		if (frame->IsHidden())
			continue;
		// check frame inside. (visibility)
		bool covered = false;
		if (frame->InsideFrameRect(&covered, bounds, this->contentTransformInverse) == false)	// frame is not visible.
			continue;

		if (frame->RenderInternal())
			drawSelf = true;  // redraw self if child has been drawn

		if (this->enableVisibilityTest)
		{
			// check opacity for entire area. (covered area)
			if (covered && frame->blendState.dstBlendRGB == DKBlendState::BlendModeZero && frame->blendState.dstBlendAlpha == DKBlendState::BlendModeZero)
			{
				if (drawSelf || this->drawSurface)
				{
					// no need to draw the rest. (no longer visible)
					// display drawn frames with inversed order, and return.
					for (long i = index; i >= 0; --i)
					{
						frame = subframes.Value(i);
						if (frame->IsHidden())
							continue;
						DKASSERT_DEBUG(frame->Texture() != NULL);
						renderer->RenderTexturedRect(DKRect(0,0,1,1), frame->Transform(), DKRect(0,0,1,1), DKMatrix3::identity, frame->Texture(), NULL, frame->color, frame->blendState);
					}
					drawSurface = false;
					return true;
				}
				return false;
			}
		}
	}

	if (drawSelf || this->drawSurface)
	{
		OnRender(*renderer);		
		drawSurface = false;	

		for (long i = (long)subframes.Count() - 1; i >= 0; --i)
		{
			const DKFrame* frame = subframes.Value(i);
			if (frame->IsHidden())
				continue;
			renderer->RenderTexturedRect(DKRect(0,0,1,1), frame->Transform(), DKRect(0,0,1,1), DKMatrix3::identity, frame->Texture(), NULL, frame->color, frame->blendState);
		}
		return true;
	}
	return false;
}

void DKFrame::Update(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
	DKASSERT_DESC_DEBUG(IsLoaded(), "Frame must be initialized with screen!");

	OnUpdate(tickDelta, tick, tickDate);

	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		subFramesCopy.Value(index)->Update(tickDelta, tick, tickDate);
	}
}

void DKFrame::SetColor(const DKColor& color)
{
	if (screen && screen->RootFrame() == this)
	{
		DKLog("RootFrame's color cannot be changed.\n");
	}
	else
	{
		this->color = color.RGBA32Value();
		if (this->superframe)
			superframe->SetRedraw();
	}
}

DKColor DKFrame::Color(void) const
{
	return color;
}

void DKFrame::SetBlendState(const DKBlendState& blend)
{
	if (screen && screen->RootFrame() == this)
	{
		DKLog("RootFrame's blend state cannot be changed.\n");
	}
	else
	{
		this->blendState = blend;
		if (this->superframe)
			superframe->SetRedraw();
	}
}

const DKBlendState& DKFrame::BlendState(void) const
{
	return blendState;
}

bool DKFrame::ProcessKeyboardEvent(DKWindow::EventKeyboard type, int deviceId, DKVirtualKey key, const DKString& text)
{
	struct EventParam
	{
		DKWindow::EventKeyboard type;
		int deviceId;
		DKVirtualKey key;
		const DKString& text;
	} ep = {type, deviceId, key, text};

	// call PreprocessKeyboardEvent() from top level object to this.
	// if function returns true, stop precessing. (intercepted)
	struct Preprocess
	{
		DKFrame* frame;
		bool operator () (EventParam& ep)
		{
			if (frame->superframe)
			{
				Preprocess parent = { frame->superframe };
				if (parent(ep))
					return true;
			}
			return frame->PreprocessKeyboardEvent(ep.type, ep.deviceId, ep.key, ep.text);
		}
	};

	Preprocess pre = { this };
	if (pre(ep))
		return true;

	if (this->CanHandleKeyboard())
	{
		switch (type)
		{
		case DKWindow::EventKeyboardDown:
			OnKeyDown(deviceId, key);
			break;
		case DKWindow::EventKeyboardUp:
			OnKeyUp(deviceId, key);
			break;
		case DKWindow::EventKeyboardTextInput:
			OnTextInput(deviceId, text);
			break;
		case DKWindow::EventKeyboardTextInputCandidate:
			OnTextInputCandidate(deviceId, text);
			break;
		}
		return true;
	}
	return false;
}

bool DKFrame::ProcessMouseEvent(DKWindow::EventMouse type, int deviceId, int buttonId, const DKPoint& pos, const DKVector2& delta, bool propagate)
{
	// apply content scale, transform with normalized coordinates. (pos is normalized)
	DKVector2 localPos = DKVector2(pos.x * this->contentScale.width, pos.y * this->contentScale.height);
	localPos.Transform(this->contentTransformInverse);

	DKVector2 localPosOld = DKVector2((pos.x - delta.x) * this->contentScale.width, (pos.y - delta.y) * this->contentScale.height);
	localPosOld.Transform(this->contentTransformInverse);

	DKVector2 localDelta = localPos - localPosOld;

	// call PreprocessMouseEvent() from top level object to this.
	// if function returns true, stop precessing. (intercepted)
	struct Preprocess
	{
		struct Param
		{
			DKWindow::EventMouse type;
			int deviceId;
			int buttonId;
		};
		DKFrame* frame;
		bool operator () (Param& param, const DKPoint& pos, const DKPoint& posOld)
		{
			if (frame->superframe)
			{
				DKPoint pos2 = frame->LocalToSuper(pos);
				DKPoint posOld2 = frame->LocalToSuper(posOld);

				Preprocess parent = { frame->superframe };
				if (parent(param, pos2, posOld2))
					return true;
			}
			return frame->PreprocessMouseEvent(param.type, param.deviceId, param.buttonId, pos, (pos - posOld).Vector());
		}
	};

	if (propagate)
	{
		if (!this->HitTest(localPos))
			return false;

		if (this->ContentHitTest(localPos))
		{
			for (FrameArray::Index index = 0; index < subframes.Count(); index++)
			{
				DKFrame* frame = subframes.Value(index);
				if (frame->IsHidden())
					continue;

				// apply inversed frame transform (convert to normalized frame coordinates)
				DKMatrix3 tm = frame->TransformInverse();
				DKVector2 posInFrame = DKVector2(localPos).Transform(tm);

				if (DKRect(0, 0, 1, 1).IsPointInside(posInFrame))
				{
					DKVector2 oldPosInFrame = DKVector2(localPosOld).Transform(tm);
					DKVector2 deltaInFrame = posInFrame - oldPosInFrame;

					// send event to frame whether it is able to process or not. (frame is visible-destionation)
					if (frame->ProcessMouseEvent(type, deviceId, buttonId, posInFrame, deltaInFrame, propagate))
						return true;
				}
			}
		}
	}
	else
	{
		// call PreprocessMouseEvent() from top level object to this.
		// if function returns true, stop precessing. (intercepted)
		Preprocess::Param param = {type, deviceId, buttonId};
		Preprocess pre = { this };
		if (pre(param, localPos, localPosOld))
			return true;
	}
	
	// no frames can handle event. should process by this.
	// or mouse has been captured by this.
	if (this->CanHandleMouse())
	{
		if (propagate)
		{
			// call PreprocessMouseEvent() from top level object to this.
			// if function returns true, stop precessing. (intercepted)
			Preprocess::Param param = {type, deviceId, buttonId};
			Preprocess pre = { this };
			if (pre(param, localPos, localPosOld))
				return true;
		}

		switch (type)
		{
		case DKWindow::EventMouseDown:
			OnMouseDown(deviceId, buttonId, localPos);
			break;
		case DKWindow::EventMouseUp:
			OnMouseUp(deviceId, buttonId, localPos);
			break;
		case DKWindow::EventMouseMove:
			OnMouseMove(deviceId, localPos, localDelta);
			break;
		case DKWindow::EventMouseWheel:
			OnMouseWheel(deviceId, localPos, localDelta);
			break;
		}
		return true;
	}
	return false;
}

bool DKFrame::ProcessMouseInOut(int deviceId, const DKPoint& pos, bool insideParent)
{
	if (this->hidden)
		return false;

	bool insideSelf = insideParent ? DKRect(0, 0, 1, 1).IsPointInside(pos) : false;

	DKVector2 localPos = DKVector2(pos.x * this->contentScale.width, pos.y * this->contentScale.height);
	localPos.Transform(this->contentTransformInverse);

	bool hover = false;
	DKMap<int, bool>::Pair* p = mouseHover.Find(deviceId);
	if (p)
		hover = p->value;

	if (insideSelf && HitTest(localPos))
	{
		if (!hover)
		{
			hover = true;
			if (this->CanHandleMouse())
			{
				mouseHover.Update(deviceId, hover);
				OnMouseHover(deviceId);
			}
		}
	}
	else
	{
		if (hover)
		{
			hover = false;
			if (this->CanHandleMouse())
			{
				mouseHover.Update(deviceId, hover);
				OnMouseLeave(deviceId);
			}
		}
	}

	bool subframeHover = hover && this->ContentHitTest(localPos);
	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		DKFrame* frame = subFramesCopy.Value(index);
		DKMatrix3 tm = frame->TransformInverse();
		if (subFramesCopy.Value(index)->ProcessMouseInOut(deviceId, DKVector2(localPos).Transform(tm), subframeHover))
			subframeHover = false;
	}
	return hover;
}

void DKFrame::ReleaseMouseData(void)
{
	FrameArray subFramesCopy = Subframes();
	for (FrameArray::Index index = 0; index < subFramesCopy.Count(); index++)
	{
		subFramesCopy.Value(index)->ReleaseMouseData();
	}

	mouseHover.EnumerateForward([this](DKMap<int, bool>::Pair& pair)
	{
		if (pair.value)
			OnMouseLeave(pair.key);		// mouse leaved
	});
	mouseHover.Clear();
}

void DKFrame::SetHidden(bool hidden)
{
	if (screen && screen->RootFrame() == this)
	{
		DKLog("RootFrame is always visible.\n");
		return;
	}

	if (this->hidden != hidden)
	{
		this->hidden = hidden;
		if (this->hidden)
		{
			ReleaseMouseData();
		}
		if (this->superframe)
			superframe->SetRedraw();
	}
}

void DKFrame::SetEnabled(bool enabled)
{
	if (this->enabled != enabled)
	{
		this->enabled = enabled;
		if (!this->enabled)
		{
			ReleaseMouseData();
		}
		SetRedraw();
	}
}

bool DKFrame::CanHandleKeyboard(void) const
{
	return this->IsEnabled();
}

bool DKFrame::CanHandleMouse(void) const
{
	return (this->IsEnabled() && this->IsVisibleOnScreen());
}

bool DKFrame::IsVisibleOnScreen(void) const
{
	if (screen == NULL)
		return false;

	if (screen->RootFrame() == this)
		return true;

	if (this->hidden)
		return false;

	if (superframe)
		return superframe->IsVisibleOnScreen();

	return false;
}

void DKFrame::SetDepthFormat(DKRenderTarget::DepthFormat fmt)
{
	if (screen && screen->RootFrame() == this)
	{
		DKLog("RootFrame's depth format cannot be changed.\n");
	}
	else
	{
		if (depthFormat != fmt)
		{
			depthFormat = fmt;
			renderer = NULL;		// create on render
			SetRedraw();
		}
	}
}

DKRenderTarget::DepthFormat DKFrame::DepthFormat(void) const
{
	return depthFormat;
}

void DKFrame::DiscardSurface(void)
{
	renderer = NULL;		// create on render
	SetRedraw();
}

void DKFrame::SetSurfaceVisibilityTest(bool enable)
{
	this->enableVisibilityTest = enable;
}

bool DKFrame::IsSurfaceVisibilityTestEnabled(void) const
{
	return this->enableVisibilityTest;
}

void DKFrame::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(1,1,1,1));
}
