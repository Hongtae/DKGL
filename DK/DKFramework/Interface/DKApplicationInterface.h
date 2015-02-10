//
//  File: DKApplicationInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013-2014 Hongtae Kim. All rights reserved.
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

namespace DKFramework
{
	class DKApplicationInterface
	{
	public:
		using SystemPath = DKApplication::SystemPath;

		virtual ~DKApplicationInterface(void) {}

		virtual int Run(DKFoundation::DKArray<char*>& args) = 0;
		virtual void Terminate(int exitCode) = 0;

		virtual DKFoundation::DKLogger& DefaultLogger(void) = 0;
		virtual DKFoundation::DKString EnvironmentPath(SystemPath) = 0;
		virtual DKFoundation::DKString ModulePath(void) = 0;

		virtual DKFoundation::DKObject<DKFoundation::DKData> LoadResource(const DKFoundation::DKString& res, DKFoundation::DKAllocator& alloc) = 0;		// read-writable
		virtual DKFoundation::DKObject<DKFoundation::DKData> LoadStaticResource(const DKFoundation::DKString& res) = 0;	// read-only

		virtual DKRect DisplayBounds(int displayId) const = 0;
		virtual DKRect ScreenContentBounds(int displayId) const = 0;

		virtual DKFoundation::DKString HostName(void) const = 0;
		virtual DKFoundation::DKString OSName(void) const = 0;
		virtual DKFoundation::DKString UserName(void) const = 0;

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
