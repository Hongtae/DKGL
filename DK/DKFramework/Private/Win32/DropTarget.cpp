//
//  File: DropTarget.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include "DropTarget.h"

using namespace DKFramework;
using namespace DKFramework::Private::Win32;

DropTarget::DropTarget(DKWindow* win)
	: refCount(1)
	, target(win)
	, periodicUpdate(FALSE)
	, lastEffectMask(DROPEFFECT_NONE)
{
}

DropTarget::~DropTarget(void)
{
}

// *** IUnknown ***
HRESULT DropTarget::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown || riid == IID_IDropTarget) {
		*ppv = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG DropTarget::AddRef()
{
	return InterlockedIncrement(&refCount);
}

ULONG DropTarget::Release()
{
	LONG cRef = InterlockedDecrement(&refCount);
	if (cRef == 0)
	{
		delete this;
	}
	return cRef;
}

HRESULT DropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	source.Clear();
	dropAllowed = FALSE;	
	lastEffectMask = DROPEFFECT_NONE;

	if (target->Callback().draggingFeedback)
	{
		FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		if (pDataObject->QueryGetData(&fmtetc) == S_OK)
		{
			source = FilesFromDataObject(pDataObject);
			if (source.Count() > 0)
				dropAllowed = TRUE;
		}
	}
	
	if(dropAllowed)
	{
		POINT pt2 = { pt.x,  pt.y };
		ScreenToClient((HWND)target->PlatformHandle(), &pt2);
		pt = { pt2.x, pt2.y };

		lastKeyState = grfKeyState;
		lastPosition = pt2;

		DKWindow::DraggingState state = DKWindow::DraggingEntered;
		DKWindow::DragOperation op = target->Callback().draggingFeedback->Invoke(target,
																				 state,
																				 DKPoint(pt2.x, pt2.y),
																				 source);
		switch (op)
		{
		case DKWindow::DragOperationCopy:
			lastEffectMask = DROPEFFECT_COPY;
			break;
		case DKWindow::DragOperationMove:
			lastEffectMask = DROPEFFECT_MOVE;
			break;
		case DKWindow::DragOperationLink:
			lastEffectMask = DROPEFFECT_LINK;
			break;
		default:
			lastEffectMask = DROPEFFECT_NONE;
			break;
		}
		*pdwEffect = (*pdwEffect) & lastEffectMask;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

HRESULT DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(dropAllowed)
	{
		POINT pt2 = { pt.x,  pt.y };
		ScreenToClient((HWND)target->PlatformHandle(), &pt2);
		pt = { pt2.x, pt2.y };

		bool update = true;
		if (!periodicUpdate &&
			lastPosition.x == pt2.x &&
			lastPosition.y == pt2.y &&
			lastKeyState == grfKeyState)
			update = false;

		if (update)
		{
			lastKeyState = grfKeyState;
			lastPosition = pt2;

			DKWindow::DraggingState state = DKWindow::DraggingUpdated;
			DKWindow::DragOperation op = target->Callback().draggingFeedback->Invoke(target,
																					 state,
																					 DKPoint(pt2.x, pt2.y),
																					 source);
			switch (op)
			{
			case DKWindow::DragOperationCopy:
				lastEffectMask = DROPEFFECT_COPY;
				break;
			case DKWindow::DragOperationMove:
				lastEffectMask = DROPEFFECT_MOVE;
				break;
			case DKWindow::DragOperationLink:
				lastEffectMask = DROPEFFECT_LINK;
				break;
			default:
				lastEffectMask = DROPEFFECT_NONE;
				break;
			}
		}

		*pdwEffect = (*pdwEffect) & lastEffectMask;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}


	return S_OK;
}

HRESULT DropTarget::DragLeave(void)
{
	if (dropAllowed)
	{
		DKWindow::DraggingState state = DKWindow::DraggingExited;
		DKWindow::DragOperation op = target->Callback().draggingFeedback->Invoke(target,
																				 state,
																				 DKPoint(lastPosition.x, lastPosition.y),
																				 source);
	}
	source.Clear();
	source.ShrinkToFit();
	lastEffectMask = DROPEFFECT_NONE;
	return S_OK;
}

HRESULT DropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(dropAllowed)
	{
		POINT pt2 = { pt.x, pt.y };
		ScreenToClient((HWND)target->PlatformHandle(), &pt2);
		pt = { pt2.x, pt2.y };

		DKWindow::DraggingState state = DKWindow::DraggingDropped;
		DKWindow::DragOperation op = target->Callback().draggingFeedback->Invoke(target,
																				 state,
																				 DKPoint(pt2.x, pt2.y),
																				 source);
		switch (op)
		{
		case DKWindow::DragOperationCopy:
			lastEffectMask = DROPEFFECT_COPY;
			break;
		case DKWindow::DragOperationMove:
			lastEffectMask = DROPEFFECT_MOVE;
			break;
		case DKWindow::DragOperationLink:
			lastEffectMask = DROPEFFECT_LINK;
			break;
		default:
			lastEffectMask = DROPEFFECT_NONE;
			break;
		}
		*pdwEffect = (*pdwEffect) & DROPEFFECT_COPY;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	source.Clear();
	source.ShrinkToFit();
	lastEffectMask = DROPEFFECT_NONE;
	return S_OK;
}

DKStringArray DropTarget::FilesFromDataObject(IDataObject* pDataObject)
{
	DKStringArray filenames;

	FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgm;
	if (SUCCEEDED(pDataObject->GetData(&fmte, &stgm)))
	{
		HDROP hdrop = reinterpret_cast<HDROP>(stgm.hGlobal);
		UINT numFiles = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
		for (UINT i = 0; i < numFiles; ++i)
		{
			UINT len = DragQueryFileW(hdrop, i, NULL, 0);
			LPWSTR buff = (LPWSTR)DKMalloc(sizeof(WCHAR) * (len+2));
			UINT r = DragQueryFileW(hdrop, i, buff, len+1);
			buff[r] = 0;

			filenames.Add(buff);

			DKFree(buff);
		}
		ReleaseStgMedium(&stgm);
	}

	return filenames;
}

void* DropTarget::operator new (size_t s)
{
	return DKMalloc(s);
}

void DropTarget::operator delete (void* p) noexcept
{
	DKFree(p);
}

#endif // _WIN32
