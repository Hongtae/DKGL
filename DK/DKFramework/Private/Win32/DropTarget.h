//
//  File: DropTarget.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#ifdef _WIN32
#include <Windows.h>
#include "../../DKWindow.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			class DropTarget : public IDropTarget
			{
			public:
				DropTarget(DKWindow* target);
				~DropTarget();

				// *** IUnknown ***
				STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
				STDMETHODIMP_(ULONG) AddRef();
				STDMETHODIMP_(ULONG) Release();

				// *** IDropTarget ***
				STDMETHODIMP DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
				STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
				STDMETHODIMP DragLeave();
				STDMETHODIMP Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

				DKStringArray FilesFromDataObject(IDataObject*);

				void* operator new (size_t);
				void operator delete (void*) noexcept;

			private:
				POINT lastPosition;
				DWORD lastKeyState;
				DWORD lastEffectMask;
				BOOL dropAllowed;
				BOOL periodicUpdate;
				LONG refCount;
				DKStringArray source;
				DKWindow* target;
			};
		}
	}
}
#endif // _WIN32
