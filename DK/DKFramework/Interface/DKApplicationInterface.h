//
//  File: DKApplicationInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKFoundation.h"
#include "../DKApplication.h"

///////////////////////////////////////////////////////////////////////////////
// DKApplicationInterface
// An abstract class, interface for application environment controls.
// You may need to subclass for your platform, If you have plan to use
// DKApplication.
///////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKApplicationInterface
	{
	public:
		using SystemPath = DKApplication::SystemPath;

		virtual ~DKApplicationInterface(void) {}

		virtual int Run(DKArray<char*>& args) = 0;
		virtual void Terminate(int exitCode) = 0;

		virtual DKLogger& DefaultLogger(void) = 0;
		virtual DKString EnvironmentPath(SystemPath) = 0;
		virtual DKString ModulePath(void) = 0;

		virtual DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc) = 0;		// read-writable
		virtual DKObject<DKData> LoadStaticResource(const DKString& res) = 0;	// read-only

		virtual DKRect DisplayBounds(int displayId) const = 0;
		virtual DKRect ScreenContentBounds(int displayId) const = 0;

		virtual DKString HostName(void) const = 0;
		virtual DKString OSName(void) const = 0;
		virtual DKString UserName(void) const = 0;

		static DKApplicationInterface* CreateInterface(DKApplication*);

		static DKApplicationInterface* SharedInstance(void)
		{
			DKApplication* app = DKApplication::Instance();
			if (app)
				return app->impl;
			return NULL;
		}

	protected:
		static void AppInitialize(DKApplication* app)	{app->Initialize();}
		static void AppFinalize(DKApplication* app)		{app->Finalize();}
	};
}
