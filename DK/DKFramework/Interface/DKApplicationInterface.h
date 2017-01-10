//
//  File: DKApplicationInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "DKBackendInterface.h"
#include "../DKApplication.h"

namespace DKFramework
{
	/// @brief Interface for platform dependent application environment control.
	///
	/// Microsoft Windows, Apple macOS/iOS is builtin supported at this time.
	/// You may need to your own subclass for your platform if you have plan
	/// to use DKApplication.
	class DKApplicationInterface : public DKBackendInterface
	{
	public:
		using SystemPath = DKApplication::SystemPath;
		using ProcessInfo = DKApplication::ProcessInfo;

		virtual DKEventLoop* EventLoop(void) = 0;
		virtual DKLogger* DefaultLogger(void) = 0;

		virtual DKString DefaultPath(SystemPath) = 0;
		virtual DKString ProcessInfoString(ProcessInfo) = 0;

		/// Load dll/exe resource as writable copy.
		virtual DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc) = 0;		// read-writable
		/// Load dll/exe resource (read-only)
		virtual DKObject<DKData> LoadStaticResource(const DKString& res) = 0;	// read-only

		virtual DKRect DisplayBounds(int displayId) const = 0;
		virtual DKRect ScreenContentBounds(int displayId) const = 0;

		static DKApplicationInterface* CreateInterface(DKApplication*, int argc, char* argv[]);
		static DKApplicationInterface* Instance(DKApplication* app) { return app->impl; }

		static void AppInitialize(DKApplication* app)	{ app->Initialize(); }
		static void AppFinalize(DKApplication* app)		{ app->Finalize(); }
	};
}
