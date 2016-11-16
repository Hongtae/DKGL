//
//  File: DKApplicationInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKApplication.h"
#include "DKBackendInterface.h"

///////////////////////////////////////////////////////////////////////////////
// DKApplicationInterface
// An abstract class, interface for application environment controls.
// You may need to subclass for your platform, If you have plan to use
// DKApplication.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKApplicationInterface : public DKBackendInterface
	{
	public:
		using SystemPath = DKApplication::SystemPath;
		using ProcessInfo = DKApplication::ProcessInfo;

		virtual DKEventLoop* EventLoop(void) = 0;
		virtual DKLogger* DefaultLogger(void) = 0;

		virtual DKString DefaultPath(SystemPath) = 0;
		virtual DKString ProcessInfoString(ProcessInfo) = 0;

		virtual DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc) = 0;		// read-writable
		virtual DKObject<DKData> LoadStaticResource(const DKString& res) = 0;	// read-only

		virtual DKRect DisplayBounds(int displayId) const = 0;
		virtual DKRect ScreenContentBounds(int displayId) const = 0;

		static DKApplicationInterface* CreateInterface(DKApplication*, int argc, char* argv[]);

		static DKApplicationInterface* SharedInstance(void)
		{
			DKApplication* app = DKApplication::Instance();
			if (app)
				return app->impl;
			return NULL;
		}

		static void AppInitialize(DKApplication* app)	{ app->Initialize(); }
		static void AppFinalize(DKApplication* app)		{ app->Finalize(); }
	};
}
