#include "DCObject.h"

#define DCOBJECT_TABLE_SIZE 7
static DKMap<const void*, PyObject*> objectMap[DCOBJECT_TABLE_SIZE];

//#define PYDK_MULTITHREADED
#ifdef PYDK_MULTITHREADED
static DKSpinLock objectMapLock[DCOBJECT_TABLE_SIZE];
#else
struct {const void* key; PyObject* value;} cachedObject = {NULL, NULL};
#endif

PyObject* DCObjectFromAddress(const void* addr)
{
#ifndef PYDK_MULTITHREADED
	if (addr == cachedObject.key)
		return cachedObject.value;
#endif
	PyObject* value = NULL;
	size_t n = reinterpret_cast<uintptr_t>(addr) % DCOBJECT_TABLE_SIZE;
#ifdef PYDK_MULTITHREADED
	DKCriticalSection<DKSpinLock> guard(objectMapLock[n]);
#endif
	auto p = objectMap[n].Find(addr);
	if (p)
		value = p->value;
#ifndef PYDK_MULTITHREADED
	cachedObject.key = addr;
	cachedObject.value = value;
#endif
	return value;
}

void DCObjectSetAddress(const void* addr, PyObject* obj)
{
	size_t n = reinterpret_cast<uintptr_t>(addr) % DCOBJECT_TABLE_SIZE;
#ifdef PYDK_MULTITHREADED
	DKCriticalSection<DKSpinLock> guard(objectMapLock[n]);
#endif
	if (obj)
	{
#ifdef DKLIB_DEBUG_ENABLED
		auto p = objectMap[n].Find(addr);
		if (p)
			DKLog("Warning: DCObjectSetAddress(%x) already exist!\n", addr);
#endif
		objectMap[n].Update(addr, obj);
	}
	else
	{
		objectMap[n].Remove(addr);
	}
#ifndef PYDK_MULTITHREADED
	cachedObject.key = addr;
	cachedObject.value = obj;
#endif
}

// 사용자 정의 Type-Object.
struct DefaultClass
{
	PyTypeObject* baseType;
	PyTypeObject* defaultType;
};
static DKArray<DefaultClass> defaultClasses;
#ifdef PYDK_MULTITHREADED
static DKSpinLock defaultClassesLock;
#else
DefaultClass* cachedUserType = NULL;
#endif

PyObject* DCObjectCreateDefaultClass(PyTypeObject* baseType, PyObject* args, PyObject* kwds)
{
	DKASSERT_DEBUG(baseType);
	PyTypeObject* tp = DCObjectDefaultClass(baseType);
	DKASSERT_DEBUG(tp);
	DKASSERT_DEBUG(PyType_IsSubtype(tp, baseType));
	// 객체 생성.
	PyObject* ret = tp->tp_new(tp, args, kwds);
	Py_DECREF(tp);
	return ret;
}

bool DCObjectSetDefaultClass(PyTypeObject* baseType, PyTypeObject* defaultType)
{
	DKASSERT_DEBUG(baseType);

	if (baseType == defaultType)
		defaultType = NULL;

	if (defaultType && !PyType_IsSubtype(defaultType, baseType))	// wrong type!
		return false;

#ifdef PYDK_MULTITHREADED
	DKCriticalSection<DKSpinLock> guard(defaultClassesLock);
#else
	if (cachedUserType && cachedUserType->baseType == baseType)
	{
		Py_XDECREF(cachedUserType->defaultType);
		cachedUserType->defaultType = defaultType;
		Py_XINCREF(cachedUserType->defaultType);

#ifdef DKLIB_DEBUG_ENABLED
		if (defaultType)
			DKLog("DCObjectSetDefaultClass: %s -> %s\n", baseType->tp_name, defaultType->tp_name);
		else
			DKLog("DCObjectSetDefaultClass: %s restored.\n", baseType->tp_name);
#endif
		return true;
	}
#endif
	for (DefaultClass& uto : defaultClasses)
	{
		if (uto.baseType == baseType)
		{
			if (uto.defaultType != defaultType)
			{
#ifndef PYDK_MULTITHREADED
				cachedUserType = &uto;
#endif
				Py_XDECREF(uto.defaultType);
				uto.defaultType = defaultType;
				Py_XINCREF(uto.defaultType);

				if (uto.defaultType)
				{
					DKASSERT_DEBUG(PyType_IsSubtype(uto.defaultType, uto.baseType));
				}
			}
#ifdef DKLIB_DEBUG_ENABLED
			if (defaultType)
				DKLog("DCObjectSetDefaultClass: %s -> %s\n", baseType->tp_name, defaultType->tp_name);
			else
				DKLog("DCObjectSetDefaultClass: %s restored.\n", baseType->tp_name);
#endif
			return true;
		}
	}
	return false;
}

PyTypeObject* DCObjectDefaultClass(PyTypeObject* baseType)
{
	DKASSERT_DEBUG(baseType);

	PyTypeObject* tp = NULL;

#ifndef PYDK_MULTITHREADED
	if (cachedUserType && cachedUserType->baseType == baseType)
	{
		if (cachedUserType->defaultType)
			tp = cachedUserType->defaultType;
		else
			tp = cachedUserType->baseType;
		DKASSERT_DEBUG(tp);
		Py_INCREF(tp);
	}
#endif
	if (tp == NULL)
	{
#ifdef PYDK_MULTITHREADED
		DKCriticalSection<DKSpinLock> guard(defaultClassesLock);
#endif
		for (DefaultClass& uto : defaultClasses)
		{
			if (uto.baseType == baseType)
			{
#ifndef PYDK_MULTITHREADED
				cachedUserType = &uto;
#endif
				if (uto.defaultType)
				{
					DKASSERT_DEBUG(PyType_IsSubtype(uto.defaultType, uto.baseType));
					tp = uto.defaultType;
				}
				else
				{
					tp = uto.baseType;
				}
				DKASSERT_DEBUG(tp);
				Py_INCREF(tp);
				break;
			}
		}
	}
	if (tp == NULL)
	{
		DKLog("Warning: TypeObject(%s) not found!\n", tp->tp_name);
		tp = baseType;
		Py_INCREF(tp);
	}
	return tp;
}

size_t DCObjectInitDefaultClasses(size_t count, DKInvocation<PyTypeObject*>* inv)
{
#ifdef PYDK_MULTITHREADED
	DKCriticalSection<DKSpinLock> guard(defaultClassesLock);
#else
	cachedUserType = NULL;
#endif
	for (DefaultClass& uto : defaultClasses)
	{
		Py_XDECREF(uto.defaultType);
	}
	defaultClasses.Clear();
	defaultClasses.Reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		PyTypeObject* tp = inv->Invoke();
		if (tp == NULL)
			break;

		DefaultClass uto = {tp, NULL};
		defaultClasses.Add(uto);
	}
	return defaultClasses.Count();
}

size_t DCObjectCount(void)
{
#ifdef PYDK_MULTITHREADED
	DKCriticalSection<DKSpinLock> guard(defaultClassesLock);
#endif
	size_t count = 0;
	for (size_t i = 0; i < DCOBJECT_TABLE_SIZE; ++i)
		count += objectMap[i].Count();
	return count;
}

PyObject* DCObjectMethodNone(PyObject*, PyObject*)
{
	Py_RETURN_NONE;
}

PyObject* DCObjectMethodTrue(PyObject*, PyObject*)
{
	Py_RETURN_TRUE;
}

PyObject* DCObjectMethodFalse(PyObject*, PyObject*)
{
	Py_RETURN_FALSE;
}

PyObject* DCObjectMethodZero(PyObject*, PyObject*)
{
	return PyLong_FromLong(0);
}
