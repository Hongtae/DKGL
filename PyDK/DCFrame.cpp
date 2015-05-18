#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"


class DCLocalFrame : public DKFrame
{
public:
	DCLocalFrame(PyObject* obj)
	: object(obj), renderer(NULL)
	{
		DKASSERT_DEBUG(object);
	}
	~DCLocalFrame(void)
	{
		if (Py_IsInitialized())
			Py_XDECREF(renderer);
	}
	void OnRender(DKRenderer& renderer) const override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* old = this->renderer;
				this->renderer = DCRendererFromObject(&renderer);
				DKASSERT_DEBUG(this->renderer);
				Py_XDECREF(old);

				Py_ssize_t rc = this->renderer->ob_refcnt;
				
				PyObject* ret = PyObject_CallMethod(object, "onRender", "O", this->renderer);
				Py_XDECREF(ret);

				if (rc != this->renderer->ob_refcnt)
				{
#ifdef DKLIB_DEBUG_ENABLED
					DKLog("Warning: frame.onRenderer holds renderer's ref-count! (%ld -> %ld)\n",
						  (long)rc,
						  (long)this->renderer->ob_refcnt);
#endif
				}
			});
		}
		else
		{
			renderer.Clear(DKColor(0,0,0,1));
		}
	}
	void OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				if (PyDateTimeAPI == NULL)
					PyDateTime_IMPORT;
				
				PyObject* date = PyDateTime_FromDateAndTime(tickDate.Year(),
															tickDate.Month(),
															tickDate.Day(),
															tickDate.Hour(),
															tickDate.Minute(),
															tickDate.Second(),
															tickDate.Microsecond());
				
				PyObject* ret = PyObject_CallMethod(object, "onUpdate", "dkO", tickDelta, tick, date);
				Py_XDECREF(ret);
				Py_XDECREF(date);
			});
		}
	}
	void OnLoaded(void) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onLoaded", NULL);
				Py_XDECREF(ret);
			});
		}
	}
	void OnUnload(void) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([this]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onUnload", NULL);
				Py_XDECREF(ret);
				Py_CLEAR(this->renderer);
			});
		}
	}
	void OnContentResized(void) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([this]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onResized", NULL);
				Py_XDECREF(ret);
			});
		}
	}
	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* po = DCPointFromObject((DKPoint*)&pos);
				PyObject* ret = PyObject_CallMethod(object, "onMouseDown", "iiO", deviceId, buttonId, po);
				Py_XDECREF(ret);
				Py_DECREF(po);
			});
		}
	}
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* pos2 = DCPointFromObject((DKPoint*)&pos);
				PyObject* ret = PyObject_CallMethod(object, "onMouseUp", "iiO", deviceId, buttonId, pos2);
				Py_XDECREF(ret);
				Py_DECREF(pos2);
			});
		}
	}
	void OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* po = DCPointFromObject((DKPoint*)&pos);
				PyObject* dt = DCVector2FromObject((DKVector2*)&delta);
				PyObject* ret = PyObject_CallMethod(object, "onMouseMove", "iOO", deviceId, po, dt);
				Py_XDECREF(ret);
				Py_DECREF(po);
				Py_DECREF(dt);
			});
		}
	}
	void OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* po = DCPointFromObject((DKPoint*)&pos);
				PyObject* dt = DCVector2FromObject((DKVector2*)&delta);
				PyObject* ret = PyObject_CallMethod(object, "onMouseWheel", "iOO", deviceId, po, dt);
				Py_XDECREF(ret);
				Py_DECREF(po);
				Py_DECREF(dt);
			});
		}
	}
	void OnMouseHover(int deviceId) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onMouseHover", "i", deviceId);
				Py_XDECREF(ret);
			});
		}
	}
	void OnMouseLeave(int deviceId) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onMouseLeave", "i", deviceId);
				Py_XDECREF(ret);
			});
		}
	}
	void OnMouseLost(int deviceId) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onMouseLost", "i", deviceId);
				Py_XDECREF(ret);
			});
		}
	}
	void OnKeyDown(int deviceId, DKVirtualKey key) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onKeyDown", "ii", deviceId, key);
				Py_XDECREF(ret);
			});
		}
	}
	void OnKeyUp(int deviceId, DKVirtualKey key) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onKeyUp", "ii", deviceId, key);
				Py_XDECREF(ret);
			});
		}
	}
	void OnTextInput(int deviceId, const DKString& str) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onTextInput", "is", deviceId, (const char*)DKStringU8(str));
				Py_XDECREF(ret);
			});
		}
	}
	void OnTextInputCandidate(int deviceId, const DKString& str) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onTextInputCandidate", "is", deviceId, (const char*)DKStringU8(str));
				Py_XDECREF(ret);
			});
		}
	}
	void OnKeyboardLost(int deviceId) override
	{
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "onKeyboardLost", "i", deviceId);
				Py_XDECREF(ret);
			});
		}
	}
	DKSize QueryContentResolution(void) const override
	{
		DKSize size(-1, -1);
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "getResolution", NULL);
				
				if (!DCSizeConverter(ret, &size))
				{
					PyErr_Clear();
					PyErr_SetString(PyExc_TypeError, "Frame.getResolution must return Size object.");
				}
				Py_XDECREF(ret);
			});
		}
		if (size.width >= 0 && size.height >= 0)
			return size;
		return DKFrame::QueryContentResolution();
	}
	bool HitTest(const DKPoint& pt) const override
	{
		int result = -1;
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "hitTest", "N", DCPointFromObject((DKPoint*)&pt));
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
			});
		}
		if (result < 0)
			return DKFrame::HitTest(pt);
		return result > 0;
	}
	bool ContentHitTest(const DKPoint& pt) const override
	{
		int result = -1;
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "contentHitTest", "N", DCPointFromObject((DKPoint*)&pt));
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
			});
		}
		if (result < 0)
			return DKFrame::HitTest(pt);
		return result > 0;
	}
	bool PreprocessMouseEvent(DKWindow::EventMouse type, int deviceId, int buttonId, const DKPoint& pos, const DKVector2& delta) override
	{
		int result = -1;
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* po = DCPointFromObject((DKPoint*)&pos);
				PyObject* dt = DCVector2FromObject((DKVector2*)&delta);

				PyObject* ret = PyObject_CallMethod(object, "preprocessMouseEvent", "iiiOO", (int)type, deviceId, buttonId, po, dt);
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
				Py_DECREF(po);
				Py_DECREF(dt);
			});
		}
		if (result < 0)
			return DKFrame::PreprocessMouseEvent(type, deviceId, buttonId, pos, delta);
		return result > 0;
	}
	bool PreprocessKeyboardEvent(DKWindow::EventKeyboard type, int deviceId, DKVirtualKey key, const DKString& text) override
	{
		int result = -1;
		if (this->object && Py_IsInitialized())
		{
			DCObjectCallPyCallableGIL([&]()
			{
				PyObject* ret = PyObject_CallMethod(object, "preprocessKeyboardEvent", "iiiN", (int)type, deviceId, (int)key, PyUnicode_FromWideChar(text,-1));
				if (ret)
					result = PyObject_IsTrue(ret);
				Py_XDECREF(ret);
			});
		}
		if (result < 0)
			return DKFrame::PreprocessKeyboardEvent(type, deviceId, key, text);
		return result > 0;
	}
	
	PyObject* object = NULL;
	mutable PyObject* renderer = NULL;
};

struct DCFrame
{
	PyObject_HEAD
	DKObject<DKFrame> frame;
	PyObject* children;
	PyObject* renderer;

	void UpdateChildren(void)
	{
		DKASSERT_DEBUG(this->frame);

		DKFrame::FrameArray subframes = this->frame->Subframes();
		size_t num = subframes.Count();
		PyObject* tuple = PyTuple_New(num);
		for (size_t i = 0; i < num; ++i)
		{
			DKFrame* child = subframes.Value(i);
			DKASSERT_DEBUG(child);
			PyObject* obj = DCFrameFromObject(child);

			DKASSERT_DEBUG(obj);
			DKASSERT_DEBUG(DCFrameToObject(obj) == child);

			PyTuple_SET_ITEM(tuple, i, obj);
		}

		Py_XDECREF(this->children);
		this->children = tuple;
	}
};

static PyObject* DCFrameNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCFrame* self = (DCFrame*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->frame) DKObject<DKFrame>();
		self->frame = NULL;
		self->children = NULL;
		self->renderer = NULL;
		return (PyObject*)self;
	}
	return NULL;
}

static int DCFrameInit(DCFrame *self, PyObject *args, PyObject *kwds)
{
	if (self->frame == NULL)
	{
		self->frame = DKOBJECT_NEW DCLocalFrame((PyObject*)self);
		DCObjectSetAddress(self->frame, (PyObject*)self);
	}
	self->UpdateChildren();
	return 0;
}

static int DCFrameClear(DCFrame* self)
{
	if (self->frame)
	{
		// 가비지 콜렉터에 의해서 제거될때는 부모 프레임에 붙어있을 수 있다. 명시적으로 제거함.
		self->frame->RemoveFromSuperframe();
		DCLocalFrame* localFrame = self->frame.SafeCast<DCLocalFrame>();
		if (localFrame)
		{
			DKASSERT_DEBUG(localFrame->IsLoaded() == false);
		}
	}

	Py_CLEAR(self->children);
	Py_CLEAR(self->renderer);

	return 0;
}

static int DCFrameTraverse(DCFrame* self, visitproc visit, void *arg)
{
	Py_VISIT(self->children);
	Py_VISIT(self->renderer);
	return 0;
}

static void DCFrameDealloc(DCFrame* self)
{
	DCFrameClear(self);
	if (self->frame)
	{
		DCObjectSetAddress(self->frame, NULL);
		self->frame = NULL;
	}
	self->frame.~DKObject<DKFrame>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCFrameScreen(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	return DCScreenFromObject(self->frame->Screen());
}

static PyObject* DCFrameParent(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKFrame* parent = self->frame->Superframe();
	return DCFrameFromObject(parent);
}

static PyObject* DCFrameChildren(DCFrame* self, PyObject*)
{
	if (self->frame)
	{
		self->UpdateChildren();
		if (self->children)
		{
			Py_INCREF(self->children);
			return self->children;
		}
	}
	return PyTuple_New(0);
}

static PyObject* DCFrameAddChild(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* child = NULL;
	if (!PyArg_ParseTuple(args, "O", &child))
		return NULL;

	DKFrame* frame = DCFrameToObject(child);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Frame object.");
		return NULL;
	}

	bool result = self->frame->AddSubframe(frame);
	if (result)
	{
		self->UpdateChildren();
		Py_CLEAR(self->renderer);
		Py_RETURN_NONE;
	}
	PyErr_SetString(PyExc_ValueError, "child object cannot be added.");
	return NULL;
}

static PyObject* DCFrameRemoveFromParent(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	Py_CLEAR(self->renderer);

	DKFrame* super = self->frame->Superframe();
	if (super)
	{
		self->frame->RemoveFromSuperframe();

		PyObject* obj = DCObjectFromAddress(super);
		if (obj)
		{
			Py_INCREF(obj);
			DKASSERT_DEBUG(DCFrameToObject(obj) == super);
			((DCFrame*)obj)->UpdateChildren();
			Py_DECREF(obj);
		}
	}
	Py_RETURN_NONE;
}

static PyObject* DCFrameBringChildToFront(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* child = NULL;
	if (!PyArg_ParseTuple(args, "O", &child))
		return NULL;

	DKFrame* frame = DCFrameToObject(child);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Frame object.");
		return NULL;
	}

	if (self->frame->BringSubframeToFront(frame))
		self->UpdateChildren();
	Py_RETURN_NONE;
}

static PyObject* DCFrameSendChildToBack(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* child = NULL;
	if (!PyArg_ParseTuple(args, "O", &child))
		return NULL;

	DKFrame* frame = DCFrameToObject(child);
	if (frame == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be Frame object.");
		return NULL;
	}

	if (self->frame->SendSubframeToBack(frame))
		self->UpdateChildren();
	Py_RETURN_NONE;
}

static PyObject* DCFrameNumberOfChildren(DCFrame* self, PyObject*)
{
	size_t num = 0;
	if (self->frame)
		num = self->frame->NumberOfSubframes();
	return PyLong_FromSize_t(num);
}

static PyObject* DCFrameNumberOfDescendants(DCFrame* self, PyObject*)
{
	size_t num = 0;
	if (self->frame)
		num = self->frame->NumberOfDescendants();
	return PyLong_FromSize_t(num);
}

static PyObject* DCFrameIsDescendantOf(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* obj = NULL;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKFrame* frame = DCFrameToObject(obj);
	if (frame)
	{
		bool result = self->frame->IsDescendantOf(frame);
		if (result) { Py_RETURN_TRUE; }
	}
	Py_RETURN_FALSE;
}

static PyObject* DCFrameIsVisibleOnScreen(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->IsVisibleOnScreen()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCFrameUpdate(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	double delta;
	unsigned PY_LONG_LONG tick;
	PyObject* dateObj = NULL;
	if (!PyArg_ParseTuple(args, "dK|O", &delta, &tick, &dateObj))
		return NULL;

	DKDateTime dateTime = DKDateTime::Now();
	if (dateObj)
	{
		if (PyDateTimeAPI == NULL)
			PyDateTime_IMPORT;

		if (!PyDateTime_Check(dateObj))
		{
			PyErr_SetString(PyExc_TypeError, "optional third argument must be datetime object.");
			return NULL;
		}
		dateTime = DKDateTime(PyDateTime_GET_YEAR(dateObj),
							  PyDateTime_GET_MONTH(dateObj),
							  PyDateTime_GET_DAY(dateObj),
							  PyDateTime_DATE_GET_HOUR(dateObj),
							  PyDateTime_DATE_GET_MINUTE(dateObj),
							  PyDateTime_DATE_GET_SECOND(dateObj),
							  PyDateTime_DATE_GET_MICROSECOND(dateObj),
							  0		// offset from UTC (초단위)
							  );
	}

	Py_BEGIN_ALLOW_THREADS
	self->frame->Update(delta, tick, dateTime);
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCFrameRender(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	Py_BEGIN_ALLOW_THREADS
	self->frame->Render();
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCFrameRedraw(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	self->frame->SetRedraw();
	Py_RETURN_NONE;
}

static PyObject* DCFrameDiscardSurface(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	self->frame->DiscardSurface();
	Py_RETURN_NONE;
}

static PyObject* DCFrameColor(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKColor c = self->frame->Color();
	return DCColorFromObject(const_cast<DKColor*>(&c));
}

static PyObject* DCFrameSetColor(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKColor c;
	if (!PyArg_ParseTuple(args, "O&", &DCColorConverter, &c))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Color object.");
		return NULL;
	}
	self->frame->SetColor(c);
	Py_RETURN_NONE;
}

static PyObject* DCFrameBlendState(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	const DKBlendState& bs = self->frame->BlendState();
	return DCBlendStateFromObject(const_cast<DKBlendState*>(&bs));
}

static PyObject* DCFrameSetBlendState(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;
	DKBlendState* bs = DCBlendStateToObject(obj);
	if (bs == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument must be BlendState object.");
		return NULL;
	}
	self->frame->SetBlendState(*bs);
	Py_RETURN_NONE;
}

static PyObject* DCFrameCalculateBestResolution(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKSize size = self->frame->DKFrame::QueryContentResolution();
	return DCSizeFromObject(&size);
}

static PyObject* DCFrameUpdateContentResolution(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	self->frame->UpdateContentResolution();
	Py_RETURN_NONE;
}

static PyObject* DCFrameLocalFromRootTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->LocalFromRootTransform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameLocalToRootTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->LocalToRootTransform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameLocalFromParentTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->LocalFromSuperTransform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameLocalToParentTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->LocalToSuperTransform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->Transform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameTransformInverseMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->TransformInverse();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameSetTransformMatrix(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	DKMatrix3 mat;
	if (!PyArg_ParseTuple(args, "O&", &DCMatrix3Converter, &mat))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Matrix3 object.");
		return NULL;
	}
	self->frame->SetTransform(mat);
	Py_RETURN_NONE;
}

static PyObject* DCFrameContentTransformMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->ContentTransform();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameContentTransformInverseMatrix(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKMatrix3 mat = self->frame->ContentTransformInverse();
	return DCMatrix3FromObject(&mat);
}

static PyObject* DCFrameSetContentTransformMatrix(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	DKMatrix3 mat;
	if (!PyArg_ParseTuple(args, "O&", &DCMatrix3Converter, &mat))
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "argument must be Matrix3 object.");
		return NULL;
	}
	self->frame->SetContentTransform(mat);
	Py_RETURN_NONE;
}

static PyObject* DCFrameConvertLocalToParent(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKPoint pt;
	if (!PyArg_ParseTuple(args, "O&", &DCPointConverter, &pt))
		return NULL;

	pt = self->frame->LocalToSuper(pt);
	return DCPointFromObject(&pt);
}

static PyObject* DCFrameConvertParentToLocal(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKPoint pt;
	if (!PyArg_ParseTuple(args, "O&", &DCPointConverter, &pt))
		return NULL;

	pt = self->frame->SuperToLocal(pt);
	return DCPointFromObject(&pt);
}

static PyObject* DCFrameConvertLocalToPixel(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKPoint* pt = DCPointToObject(obj);
	if (pt)
	{
		DKPoint pt2 = self->frame->LocalToPixel(*pt);
		return DCPointFromObject(&pt2);
	}
	DKSize* sz = DCSizeToObject(obj);
	if (sz)
	{
		DKSize sz2 = self->frame->LocalToPixel(*sz);
		return DCSizeFromObject(&sz2);
	}
	DKRect* rc = DCRectToObject(obj);
	if (rc)
	{
		DKRect rc2 = self->frame->LocalToPixel(*rc);
		return DCRectFromObject(&rc2);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Point or Size or Rect");
	return NULL;
}

static PyObject* DCFrameConvertPixelToLocal(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* obj;
	if (!PyArg_ParseTuple(args, "O", &obj))
		return NULL;

	DKPoint* pt = DCPointToObject(obj);
	if (pt)
	{
		DKPoint pt2 = self->frame->PixelToLocal(*pt);
		return DCPointFromObject(&pt2);
	}
	DKSize* sz = DCSizeToObject(obj);
	if (sz)
	{
		DKSize sz2 = self->frame->PixelToLocal(*sz);
		return DCSizeFromObject(&sz2);
	}
	DKRect* rc = DCRectToObject(obj);
	if (rc)
	{
		DKRect rc2 = self->frame->PixelToLocal(*rc);
		return DCRectFromObject(&rc2);
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "argument must be Point or Size or Rect");
	return NULL;
}

static PyObject* DCFrameCanHandleKeyboard(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->CanHandleKeyboard()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCFrameCanHandleMouse(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->CanHandleMouse()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCFrameCaptureKeyboard(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	if (!self->frame->CaptureKeyboard(deviceId))
	{
		PyErr_Format(PyExc_RuntimeError, "failed to capture keyboard:%d.", deviceId);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCFrameCaptureMouse(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	if (!self->frame->CaptureMouse(deviceId))
	{
		PyErr_Format(PyExc_RuntimeError, "failed to capture mouse:%d.", deviceId);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject* DCFrameReleaseKeyboard(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	self->frame->ReleaseKeyboard(deviceId);
	Py_RETURN_NONE;
}

static PyObject* DCFrameReleaseMouse(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	self->frame->ReleaseMouse(deviceId);
	Py_RETURN_NONE;
}

static PyObject* DCFrameIsKeyboardCapturedBySelf(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	return PyBool_FromLong(self->frame->IsKeybaordCapturedBySelf(deviceId));
}

static PyObject* DCFrameIsMouseCapturedBySelf(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	return PyBool_FromLong(self->frame->IsMouseCapturedBySelf(deviceId));
}

static PyObject* DCFrameReleaseAllKeyboardsCapturedBySelf(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	self->frame->ReleaseAllKeyboardsCapturedBySelf();
	Py_RETURN_NONE;
}

static PyObject* DCFrameReleaseAllMiceCapturedBySelf(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	self->frame->ReleaseAllMiceCapturedBySelf();
	Py_RETURN_NONE;
}

static PyObject* DCFrameMousePosition(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	DKPoint pt = self->frame->MousePosition(deviceId);
	return DCPointFromObject(&pt);
}

static PyObject* DCFrameIsMouseHover(DCFrame* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	int deviceId = 0;
	if (!PyArg_ParseTuple(args, "i", &deviceId))
		return NULL;

	return PyBool_FromLong(self->frame->IsMouseHover(deviceId));
}

static PyObject* DCFrameBounds(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKRect rc = self->frame->Bounds();
	return DCRectFromObject(&rc);
}

static PyObject* DCFrameDisplayBounds(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKRect rc = self->frame->DisplayBounds();
	return DCRectFromObject(&rc);
}

static PyObject* DCFrameLoad(DCFrame* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	PyObject* screen = NULL;
	DKSize resolution;

	char* kwlist[] = { "screen", "resolution", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO&", kwlist, &screen, &DCSizeConverter, &resolution))
		return NULL;

	DKScreen* s = DCScreenToObject(screen);
	if (s == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "argument 'screen' must be Screen object.");
		return NULL;
	}
	Py_BEGIN_ALLOW_THREADS
	self->frame->Load(s, resolution);
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCFrameUnload(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	Py_BEGIN_ALLOW_THREADS
	self->frame->Unload();
	Py_END_ALLOW_THREADS
	Py_RETURN_NONE;
}

static PyObject* DCFrameIsLoaded(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->IsLoaded()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyObject* DCFrameIsProxy(DCFrame* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DCLocalFrame* frm = self->frame.SafeCast<DCLocalFrame>();
	if (frm)
	{
		Py_RETURN_TRUE;
	}
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "screen", (PyCFunction)&DCFrameScreen, METH_NOARGS },
	{ "parent", (PyCFunction)&DCFrameParent, METH_NOARGS },
	{ "children", (PyCFunction)&DCFrameChildren, METH_NOARGS },
	{ "addChild", (PyCFunction)&DCFrameAddChild, METH_VARARGS },
	{ "removeFromParent", (PyCFunction)&DCFrameRemoveFromParent, METH_NOARGS },
	{ "bringChildToFront", (PyCFunction)&DCFrameBringChildToFront, METH_VARARGS },
	{ "sendChildToBack", (PyCFunction)&DCFrameSendChildToBack, METH_VARARGS },
	{ "numberOfChildren", (PyCFunction)&DCFrameNumberOfChildren, METH_NOARGS },
	{ "numberOfDescendants", (PyCFunction)&DCFrameNumberOfDescendants, METH_NOARGS },
	{ "isDescendantOf", (PyCFunction)&DCFrameIsDescendantOf, METH_VARARGS },

	{ "isVisibleOnScreen", (PyCFunction)&DCFrameIsVisibleOnScreen, METH_NOARGS },
	{ "update", (PyCFunction)&DCFrameUpdate, METH_VARARGS },
	{ "render", (PyCFunction)&DCFrameRender, METH_NOARGS },
	{ "redraw", (PyCFunction)&DCFrameRedraw, METH_NOARGS },
	{ "discardSurface", (PyCFunction)&DCFrameDiscardSurface, METH_NOARGS, "recreate renderer." },
	{ "color", (PyCFunction)&DCFrameColor, METH_NOARGS },
	{ "setColor", (PyCFunction)&DCFrameSetColor, METH_VARARGS },
	{ "blendState", (PyCFunction)&DCFrameBlendState, METH_NOARGS },
	{ "setBlendState", (PyCFunction)&DCFrameSetBlendState, METH_VARARGS },

	{ "calculateBestResolution", (PyCFunction)&DCFrameCalculateBestResolution, METH_NOARGS },
	{ "getResolution", (PyCFunction)&DCFrameCalculateBestResolution, METH_NOARGS },
	{ "updateContentResolution", (PyCFunction)&DCFrameUpdateContentResolution, METH_NOARGS },

	{ "localFromRootTransformMatrix", (PyCFunction)&DCFrameLocalFromRootTransformMatrix, METH_NOARGS },
	{ "localToRootTransformMatrix", (PyCFunction)&DCFrameLocalToRootTransformMatrix, METH_NOARGS },
	{ "localFromParentTransformMatrix", (PyCFunction)&DCFrameLocalFromParentTransformMatrix, METH_NOARGS },
	{ "localToParentTransformMatrix", (PyCFunction)&DCFrameLocalToParentTransformMatrix, METH_NOARGS },

	{ "transformMatrix", (PyCFunction)&DCFrameTransformMatrix, METH_NOARGS },
	{ "transformInverseMatrix", (PyCFunction)&DCFrameTransformInverseMatrix, METH_NOARGS },
	{ "setTransformMatrix", (PyCFunction)&DCFrameSetTransformMatrix, METH_VARARGS },

	{ "contentTransformMatrix", (PyCFunction)&DCFrameContentTransformMatrix, METH_NOARGS },
	{ "contentTransformInverseMatrix", (PyCFunction)&DCFrameContentTransformInverseMatrix, METH_NOARGS },
	{ "setContentTransformMatrix", (PyCFunction)&DCFrameSetContentTransformMatrix, METH_VARARGS },

	{ "convertLocalToParent", (PyCFunction)&DCFrameConvertLocalToParent, METH_VARARGS },
	{ "convertParentToLocal", (PyCFunction)&DCFrameConvertParentToLocal, METH_VARARGS },
	{ "convertLocalToPixel", (PyCFunction)&DCFrameConvertLocalToPixel, METH_VARARGS },
	{ "convertPixelToLocal", (PyCFunction)&DCFrameConvertPixelToLocal, METH_VARARGS },

	{ "canHandleKeyboard", (PyCFunction)&DCFrameCanHandleKeyboard, METH_NOARGS },
	{ "canHandleMouse", (PyCFunction)&DCFrameCanHandleMouse, METH_NOARGS },
	{ "captureKeyboard", (PyCFunction)&DCFrameCaptureKeyboard, METH_VARARGS },
	{ "captureMouse", (PyCFunction)&DCFrameCaptureMouse, METH_VARARGS },
	{ "releaseKeyboard", (PyCFunction)&DCFrameReleaseKeyboard, METH_VARARGS },
	{ "releaseMouse", (PyCFunction)&DCFrameReleaseMouse, METH_VARARGS },
	{ "isKeyboardCapturedBySelf", (PyCFunction)&DCFrameIsKeyboardCapturedBySelf, METH_VARARGS },
	{ "isMouseCapturedBySelf", (PyCFunction)&DCFrameIsMouseCapturedBySelf, METH_VARARGS },
	{ "releaseAllKeyboardsCapturedBySelf", (PyCFunction)&DCFrameReleaseAllKeyboardsCapturedBySelf, METH_NOARGS },
	{ "releaseAllMiceCapturedBySelf", (PyCFunction)&DCFrameReleaseAllMiceCapturedBySelf, METH_NOARGS },
	{ "mousePosition", (PyCFunction)&DCFrameMousePosition, METH_VARARGS },
	{ "isMouseHover", (PyCFunction)&DCFrameIsMouseHover, METH_VARARGS },

	{ "bounds", (PyCFunction)&DCFrameBounds, METH_NOARGS },
	{ "displayBounds", (PyCFunction)&DCFrameDisplayBounds, METH_NOARGS },
	{ "load", (PyCFunction)&DCFrameLoad, METH_VARARGS | METH_KEYWORDS },
	{ "unload", (PyCFunction)&DCFrameUnload, METH_NOARGS },
	{ "isLoaded", (PyCFunction)&DCFrameIsLoaded, METH_NOARGS },

	{ "isProxy", (PyCFunction)&DCFrameIsProxy, METH_NOARGS, "determine python frame object." },

	{ "preprocessMouseEvent", &DCObjectMethodFalse, METH_VARARGS },
	{ "preprocessKeyboardEvent", &DCObjectMethodFalse, METH_VARARGS },

	{ "onRender", &DCObjectMethodNone, METH_VARARGS },
	{ "onUpdate", &DCObjectMethodNone, METH_VARARGS },
	{ "onLoaded", &DCObjectMethodNone, METH_VARARGS },
	{ "onUnload", &DCObjectMethodNone, METH_VARARGS },
	{ "onResized", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseDown", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseUp", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseMove", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseWheel", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseHover", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseLeave", &DCObjectMethodNone, METH_VARARGS },
	{ "onMouseLost", &DCObjectMethodNone, METH_VARARGS },
	{ "onKeyDown", &DCObjectMethodNone, METH_VARARGS },
	{ "onKeyUp", &DCObjectMethodNone, METH_VARARGS },
	{ "onTextInput", &DCObjectMethodNone, METH_VARARGS },
	{ "onTextInputCandidate", &DCObjectMethodNone, METH_VARARGS },
	{ "onKeyboardLost", &DCObjectMethodNone, METH_VARARGS },
	{ "hitTest", &DCObjectMethodTrue, METH_VARARGS },
	{ "contentHitTest", &DCObjectMethodTrue, METH_VARARGS },

	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCFrameHidden(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->IsHidden()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static int DCFrameSetHidden(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int h = PyObject_IsTrue(value);
	if (h < 0)
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->frame->SetHidden(h > 0);
	return 0;
}

static PyObject* DCFrameEnabled(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	if (self->frame->IsEnabled()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static int DCFrameSetEnabled(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int h = PyObject_IsTrue(value);
	if (h < 0)
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->frame->SetEnabled(h > 0);
	return 0;
}

static PyObject* DCFrameTransform(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	const DKMatrix3& mat = self->frame->Transform();
	return Py_BuildValue("fffffffff",
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2]);
}

static int DCFrameSetTransform(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix3 mat;
	if (DCMatrix3Converter(value, &mat))
	{
		self->frame->SetTransform(mat);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Matrix3.");
	return -1;
}

static PyObject* DCFrameContentScale(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	const DKSize& sz = self->frame->ContentScale();
	return Py_BuildValue("ff", sz.width, sz.height);
}

static int DCFrameSetContentScale(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKSize sz;
	if (DCSizeConverter(value, &sz))
	{
		float minScale = DKRenderer::minimumScaleFactor;
		if (sz.width < minScale || sz.height < minScale)
		{
			PyErr_SetString(PyExc_ValueError, "invalid attribute value.");
			return -1;
		}
		self->frame->SetContentScale(sz);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Size object or two floats.");
	return -1;
}

static PyObject* DCFrameContentTransform(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	const DKMatrix3& mat = self->frame->ContentTransform();
	return Py_BuildValue("fffffffff",
		mat.m[0][0], mat.m[0][1], mat.m[0][2],
		mat.m[1][0], mat.m[1][1], mat.m[1][2],
		mat.m[2][0], mat.m[2][1], mat.m[2][2]);
}

static int DCFrameSetContentTransform(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKMatrix3 mat;
	if (DCMatrix3Converter(value, &mat))
	{
		self->frame->SetContentTransform(mat);
		return 0;
	}
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Matrix3.");
	return -1;
}

static PyObject* DCFrameDepthFormat(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	return PyLong_FromLong((long)self->frame->DepthFormat());
}

static int DCFrameSetDepthFormat(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long fmt = PyLong_AsLong(value);
	if (PyErr_Occurred())
	{
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "attribute must be int.");
		return -1;
	}
	if (fmt < (long)DKRenderTarget::DepthFormatNone || fmt > (long)DKRenderTarget::DepthFormat32)
	{
		PyErr_SetString(PyExc_ValueError, "invalid depth format.");
		return -1;
	}
	self->frame->SetDepthFormat((DKRenderTarget::DepthFormat)fmt);
	return 0;
}

static PyObject* DCFrameContentResolution(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);

	DKSize sz = self->frame->ContentResolution();
	return Py_BuildValue("ff", sz.width, sz.height);
}

static PyObject* DCFrameSurfaceVisibilityTest(DCFrame* self, void*)
{
	DCOBJECT_VALIDATE(self->frame, NULL);
	return PyBool_FromLong(self->frame->IsSurfaceVisibilityTestEnabled());
}

static int DCFrameSetSurfaceVisibilityTest(DCFrame* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->frame, -1);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int h = PyObject_IsTrue(value);
	if (h < 0)
	{
		PyErr_SetString(PyExc_TypeError, "attribute must be Boolean.");
		return -1;
	}
	self->frame->SetSurfaceVisibilityTest(h > 0);
	return 0;
}

static PyGetSetDef getsets[] = {
	{ "hidden", (getter)&DCFrameHidden, (setter)&DCFrameSetHidden, 0, 0 },
	{ "enabled", (getter)&DCFrameEnabled, (setter)&DCFrameSetEnabled, 0, 0 },
	{ "transform", (getter)&DCFrameTransform, (setter)&DCFrameSetTransform, 0, 0 },
	{ "contentScale", (getter)&DCFrameContentScale, (setter)&DCFrameSetContentScale, 0, 0 },
	{ "contentTransform", (getter)&DCFrameContentTransform, (setter)&DCFrameSetContentTransform, 0, 0 },
	{ "contentResolution", (getter)&DCFrameContentResolution, 0, 0, 0 },
	{ "depthFormat", (getter)&DCFrameDepthFormat, (setter)&DCFrameSetDepthFormat, 0, 0 },
	{ "surfaceVisibilityTest", (getter)&DCFrameSurfaceVisibilityTest, (setter)&DCFrameSetSurfaceVisibilityTest, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Frame",				/* tp_name */
	sizeof(DCFrame),						/* tp_basicsize */
	0,										/* tp_itemsize */
	(destructor)&DCFrameDealloc,			/* tp_dealloc */
	0,										/* tp_print */
	0,										/* tp_getattr */
	0,										/* tp_setattr */
	0,										/* tp_reserved */
	0,										/* tp_repr */
	0,										/* tp_as_number */
	0,										/* tp_as_sequence */
	0,										/* tp_as_mapping */
	0,										/* tp_hash  */
	0,										/* tp_call */
	0,										/* tp_str */
	0,										/* tp_getattro */
	0,										/* tp_setattro */
	0,										/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT |
	Py_TPFLAGS_BASETYPE |
	Py_TPFLAGS_HAVE_GC,						/* tp_flags */
	0,										/* tp_doc */
	(traverseproc)&DCFrameTraverse,			/* tp_traverse */
	(inquiry)&DCFrameClear,					/* tp_clear */
	0,										/* tp_richcompare */
	0,										/* tp_weaklistoffset */
	0,										/* tp_iter */
	0,										/* tp_iternext */
	methods,								/* tp_methods */
	0,										/* tp_members */
	getsets,								/* tp_getset */
	0,										/* tp_base */
	0,										/* tp_dict */
	0,										/* tp_descr_get */
	0,										/* tp_descr_set */
	0,										/* tp_dictoffset */
	(initproc)&DCFrameInit,					/* tp_init */
	0,										/* tp_alloc */
	&DCFrameNew,							/* tp_new */
};

PyTypeObject* DCFrameTypeObject(void)
{
	return &objectType;
}

PyObject* DCFrameFromObject(DKFrame* frame)
{
	if (frame)
	{
		DCFrame* self = (DCFrame*)DCObjectFromAddress(frame);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			DKASSERT_DEBUG(dynamic_cast<DCLocalFrame*>(frame) == NULL);
	
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCFrame*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->frame = frame;
				DCObjectSetAddress(self->frame, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKFrame* DCFrameToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCFrame*)obj)->frame;
	}
	return NULL;
}
