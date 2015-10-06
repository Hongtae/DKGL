#include <Python.h>
#include <structmember.h>
#include <datetime.h>
#include <DK/DK.h>
#include "DCObject.h"

struct DCFont
{
	PyObject_HEAD
	DKObject<DKFont> font;
};

static PyObject* DCFontNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DCFont* self = (DCFont*)type->tp_alloc(type, 0);
	if (self)
	{
		new (&self->font) DKObject<DKFont>();
		return (PyObject*)self;
	}
	return NULL;
}

static int DCFontInit(DCFont *self, PyObject *args, PyObject *kwds)
{
	if (self->font)
		return 0;
	
	PyObject* obj = NULL;
	int point = 12;
	float embolden = 0.0f;
	float outline = 0.0f;
	DKPoint dpi = DKPoint(72, 72);
	int enableKerning = 1;
	int forceBitmap = 0;

	char* kwlist[] = { "font", "point", "embolden", "outline", "dpi", "enableKerning", "forceBitmap", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|iffO&pp", kwlist,
		&obj, &point, &embolden, &outline, &DCPointConverter, &dpi, &enableKerning, &forceBitmap))
		return -1;

	// load font from 'data' or 'file'
	if (PyUnicode_Check(obj))
	{
		const char* s = PyUnicode_AsUTF8(obj);
		if (s && s[0])
		{
			Py_BEGIN_ALLOW_THREADS

			DKString url(s);
			if (url.Find(L"://") < 0)		// file url.
			{
				self->font = DKFont::Create(url);
			}
			else
			{
				DKObject<DKBuffer> data = DKBuffer::Create(url);
				if (data)
					self->font = DKFont::Create(data);
			}

			Py_END_ALLOW_THREADS
		}
		PyErr_Clear();
	}
	if (self->font == NULL && PyObject_CheckBuffer(obj))
	{
		DKData* data = DCDataToObject(obj);
		if (data)
		{
			self->font = DKFont::Create(data);
		}
		else
		{
			Py_buffer view;
			if (PyObject_GetBuffer(obj, &view, PyBUF_SIMPLE) == 0)
			{
				self->font = DKFont::Create(view.buf, view.len);
				PyBuffer_Release(&view);
			}
		}
	}

	if (self->font)
	{
		self->font->SetStyle(point, embolden, outline, dpi, enableKerning, forceBitmap);
		DCObjectSetAddress(self->font, (PyObject*)self);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "first argument must be file path or Data object.");
	return -1;
}

static void DCFontDealloc(DCFont* self)
{
	if (self->font)
	{
		DCObjectSetAddress(self->font, NULL);
		self->font = NULL;
	}
	self->font.~DKObject<DKFont>();
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* DCFontSetStyle(DCFont* self, PyObject* args, PyObject* kwds)
{
	DCOBJECT_VALIDATE(self->font, NULL);

	int point = 12;
	float embolden = 0.0f;
	float outline = 0.0f;
	DKPoint dpi = DKPoint(72, 72);
	int enableKerning = 1;
	int forceBitmap = 0;

	char* kwlist[] = { "point", "embolden", "outline", "dpi", "enableKerning", "forceBitmap", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|ffO&p", kwlist,
		&point, &embolden, &outline, &DCPointConverter, &dpi, &enableKerning, &forceBitmap))
		return NULL;

	self->font->SetStyle(point, embolden, outline, dpi, enableKerning, forceBitmap);
	Py_RETURN_NONE;
}

static PyObject* DCFontLineHeight(DCFont* self, PyObject*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->LineHeight());
}

static PyObject* DCFontLineWidth(DCFont* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	const char* str = "";
	if (!PyArg_ParseTuple(args, "s", &str))
		return NULL;

	return PyFloat_FromDouble(self->font->LineWidth(str));
}

static PyObject* DCFontKernAdvance(DCFont* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	const char* str = NULL;
	if (!PyArg_ParseTuple(args, "s", &str))
		return NULL;

	DKString s(str);
	DKArray<DKPoint> kern;
	size_t len = s.Length();
	const wchar_t* wstr = s;
	kern.Reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		wchar_t left = wstr[i];
		wchar_t right = wstr[i+1];
		
		DKPoint pt = self->font->KernAdvance(left, right);
		kern.Add(pt);
	}

	len = kern.Count();
	PyObject* tuple = PyTuple_New(len);
	for (size_t i = 0; i < len; ++i)
	{
		DKPoint& pt = kern.Value(i);
		PyObject* obj = Py_BuildValue("ff", pt.x, pt.y);
//		PyObject* obj = DCPointFromObject(&pt);
		PyTuple_SET_ITEM(tuple, i, obj);
	}
	return tuple;
}

static PyObject* DCFontBounds(DCFont* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	const char* str = NULL;
	if (!PyArg_ParseTuple(args, "s", &str))
		return NULL;

	DKRect rc = self->font->Bounds(str);
	return DCRectFromObject(&rc);
}

static PyObject* DCFontClearCache(DCFont* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	self->font->ClearCache();
	Py_RETURN_NONE;
}

static PyObject* DCFontIsValid(DCFont* self, PyObject* args)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	if (self->font->IsValid()) { Py_RETURN_TRUE; }
	Py_RETURN_FALSE;
}

static PyMethodDef methods[] = {
	{ "setStyle", (PyCFunction)&DCFontSetStyle, METH_VARARGS | METH_KEYWORDS },
	{ "lineHeight", (PyCFunction)&DCFontLineHeight, METH_NOARGS },
	{ "lineWidth", (PyCFunction)&DCFontLineWidth, METH_VARARGS },
	{ "kernAdvance", (PyCFunction)&DCFontKernAdvance, METH_VARARGS },
	{ "bounds", (PyCFunction)&DCFontBounds, METH_VARARGS },
	{ "clearCache", (PyCFunction)&DCFontClearCache, METH_NOARGS },
	{ "isValid", (PyCFunction)&DCFontIsValid, METH_NOARGS },
	{ NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyObject* DCFontHeight(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Height());
}

static PyObject* DCFontWidth(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Width());
}

static PyObject* DCFontBaseline(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Baseline());
}

static PyObject* DCFontPointSize(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyLong_FromLong(self->font->PointSize());
}

static int DCFontSetPointSize(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	long pt = PyLong_AsLong(value);
	if (PyErr_Occurred() == NULL && pt > 0 && pt < 0x7fffffff)
	{
		int point = (int)pt;
		DKPoint dpi = self->font->Resolution();
		float embolden = self->font->Embolden();
		float outline = self->font->Outline();
		bool kerning = self->font->KerningEnabled();
		bool bitmap = self->font->ForceBitmap();

		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be positive Integer.");
	return -1;
}

static PyObject* DCFontDPI(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	const DKPoint& dpi = self->font->Resolution();
	return Py_BuildValue("ff", dpi.x, dpi.y);
}

static int DCFontSetDPI(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	DKPoint dpi;
	if (DCPointConverter(value, &dpi) && dpi.x > 0 && dpi.y > 0)
	{
		int point = self->font->PointSize();
		float embolden = self->font->Embolden();
		float outline = self->font->Outline();
		bool kerning = self->font->KerningEnabled();
		bool bitmap = self->font->ForceBitmap();
		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be positive two floats.");
	return -1;
}

static PyObject* DCFontOutline(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Outline());
}

static int DCFontSetOutline(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	float outline = PyFloat_AsDouble(value);
	if (PyErr_Occurred() == NULL && outline >= 0.0)
	{
		int point = self->font->PointSize();
		DKPoint dpi = self->font->Resolution();
		float embolden = self->font->Embolden();
		bool kerning = self->font->KerningEnabled();
		bool bitmap = self->font->ForceBitmap();

		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be positive Float number.");
	return -1;
}

static PyObject* DCFontEmbolden(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Embolden());
}

static int DCFontSetEmbolden(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	float embolden = PyFloat_AsDouble(value);
	if (PyErr_Occurred() == NULL && embolden >= 0.0)
	{
		int point = self->font->PointSize();
		DKPoint dpi = self->font->Resolution();
		float outline = self->font->Outline();
		bool kerning = self->font->KerningEnabled();
		bool bitmap = self->font->ForceBitmap();

		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be positive Float number.");
	return -1;
}

static PyObject* DCFontKerningEnabled(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->KerningEnabled());
}

static int DCFontSetKerningEnabled(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int kerning = PyObject_IsTrue(value);
	if (kerning >= 0)
	{
		int point = self->font->PointSize();
		DKPoint dpi = self->font->Resolution();
		float embolden = self->font->Embolden();
		float outline = self->font->Outline();
		bool bitmap = self->font->ForceBitmap();

		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Boolean value.");
	return -1;
}

static PyObject* DCFontForceBitmap(DCFont* self, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	return PyFloat_FromDouble(self->font->Embolden());
}

static int DCFontSetForceBitmap(DCFont* self, PyObject* value, void*)
{
	DCOBJECT_VALIDATE(self->font, NULL);
	DCOBJECT_ATTRIBUTE_NOT_DELETABLE(value);

	int forceBitmap = PyObject_IsTrue(value);
	if (forceBitmap >= 0)
	{
		int point = self->font->PointSize();
		DKPoint dpi = self->font->Resolution();
		float embolden = self->font->Embolden();
		float outline = self->font->Outline();
		bool kerning = self->font->KerningEnabled();
		bool bitmap = forceBitmap > 0;

		self->font->SetStyle(point, embolden, outline, dpi, kerning, bitmap);
		return 0;
	}

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "attribute must be Boolean value.");
	return -1;
}

static PyGetSetDef getsets[] = {
	{ "height", (getter)&DCFontHeight, 0, 0, 0 },
	{ "width", (getter)&DCFontWidth, 0, 0, 0 },
	{ "baseline", (getter)&DCFontBaseline, 0, 0, 0 },
	{ "pointSize", (getter)&DCFontPointSize, (setter)&DCFontSetPointSize, 0, 0 },
	{ "dpi", (getter)&DCFontDPI, (setter)&DCFontSetDPI, 0,0 },
	{ "outline", (getter)&DCFontOutline, (setter)&DCFontSetOutline, 0, 0 },
	{ "embolden", (getter)&DCFontEmbolden, (setter)&DCFontSetEmbolden, 0, 0 },
	{ "kerningEnabled", (getter)&DCFontKerningEnabled, (setter)&DCFontSetKerningEnabled, 0, 0 },
	{ "forceBitmap", (getter)&DCFontForceBitmap, (setter)&DCFontSetForceBitmap, 0, 0 },
	{ NULL }  /* Sentinel */
};

static PyTypeObject objectType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	PYDK_MODULE_NAME ".Font",				/* tp_name */
	sizeof(DCFont),							/* tp_basicsize */
	0,										/* tp_itemsize */
	(destructor)&DCFontDealloc,				/* tp_dealloc */
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
	Py_TPFLAGS_BASETYPE,					/* tp_flags */
	0,										/* tp_doc */
	0,										/* tp_traverse */
	0,										/* tp_clear */
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
	(initproc)&DCFontInit,					/* tp_init */
	0,										/* tp_alloc */
	&DCFontNew,								/* tp_new */
};

PyTypeObject* DCFontTypeObject(void)
{
	return &objectType;
}

PyObject* DCFontFromObject(DKFont* font)
{
	if (font)
	{
		DCFont* self = (DCFont*)DCObjectFromAddress(font);
		if (self)
		{
			Py_INCREF(self);
			return (PyObject*)self;
		}
		else
		{
			PyObject* args = PyTuple_New(0);
			PyObject* kwds = PyDict_New();

			self = (DCFont*)DCObjectCreateDefaultClass(&objectType, args, kwds);
			if (self)
			{
				self->font = font;
				DCObjectSetAddress(self->font, (PyObject*)self);
				Py_TYPE(self)->tp_init((PyObject*)self, args, kwds);
			}
			Py_XDECREF(args);
			Py_XDECREF(kwds);
			return (PyObject*)self;
		}
	}
	Py_RETURN_NONE;
}

DKFont* DCFontToObject(PyObject* obj)
{
	if (obj && PyObject_TypeCheck(obj, &objectType))
	{
		return ((DCFont*)obj)->font;
	}
	return NULL;
}
