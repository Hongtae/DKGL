//
//  File: DKWindowInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKWindow.h"

namespace DKFramework
{
	/// @brief Interface for platform dependent UI window control.
	///
	/// Microsoft Windows, Apple macOS/iOS is builtin supported at this time.
	/// You may need to your own subclass for your platform if you have plan
	/// to use DKWindow.
	class DKWindowInterface
	{
	public:
		virtual ~DKWindowInterface() {}

		using Style = DKWindow::Style;
		using MouseEvent = DKWindow::MouseEvent;
		using KeyboardEvent = DKWindow::KeyboardEvent;
		using WindowEvent = DKWindow::WindowEvent;
		
		virtual bool Create(const DKString& title, uint32_t style) = 0;
		virtual bool CreateProxy(void* systemHandle) = 0;
		virtual bool IsProxy() const = 0;
		virtual void UpdateProxy() = 0;
		virtual void Destroy() = 0;
		virtual void* PlatformHandle() const = 0;
		virtual bool IsValid() const = 0;

		virtual void ShowMouse(int deviceId, bool show) = 0;
		virtual bool IsMouseVisible(int deviceId) const = 0;
		virtual void HoldMouse(int deviceId, bool hold) = 0;
		virtual bool IsMouseHeld(int deviceId) const = 0;

		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void Activate() = 0;
		virtual void Minimize() = 0;

		/// Window's origin, size is based on system GUI coordinates.
		virtual void SetOrigin(DKPoint) = 0;
		virtual void Resize(DKSize, const DKPoint* optionalOrigin) = 0;

		virtual double ContentScaleFactor() const = 0;  ///< logical coords by pixel ratio.

		virtual void SetTitle(const DKString& title) = 0;
		virtual DKString Title() const = 0;

		virtual void SetMousePosition(int deviceId, DKPoint pt) = 0;
		virtual DKPoint MousePosition(int deviceId) const = 0;

		virtual void EnableTextInput(int deviceId, bool enable) = 0;
		virtual bool IsTextInputEnabled(int deviceId) const = 0;

		static DKWindowInterface* CreateInterface(DKWindow*);
		static DKWindowInterface* Instance(DKWindow* p) { return p->impl; }
	};
}
