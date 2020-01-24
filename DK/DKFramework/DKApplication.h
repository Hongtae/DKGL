//
//  File: DKApplication.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRect.h"
#include "DKPropertySet.h"

#ifdef _WIN32
#undef GetUserName
#endif

namespace DKFramework
{
	class DKApplicationInterface;
	/// @brief This is optional application entry point class.
	/// @details
	/// You can use this class for creating application one-sourced, cross-platform.
	/// You need subclass of this class to override application behaviors.
	/// You can have multiple instances of this class, but only one instance can run,
	/// other instances will be blocked until previous instance has finished run.
	/// This class provides retrieving process environments information.
	///
	/// Subclass note:
	/// 1. Override OnInitialize(). this is called when application to be
	///    initialized. You may need create single or multiple window(s)
	///    (as DKWindow), and screen (DKScreen) objects to attached into a window.
	///    One screen instance per one. But this is optional.
	///    You don't have to create windows, screens if your application does not
	///    need window to display. (ie console application).
	///    Whether your application has window or not, an application will have
	///    main loop (DKEventLoop) and looping till Terminate() has called.
	/// 2. Override OnTerminate(). this is called when application terminated by
	///    calling Terminate(). Your cleanup operations must be here.
	///    (destory windows, destroy screens, closing files, etc.)
	///
	/// @note
	///   Using this class is optional. You can initialize window, screen at any
	///   time. If your application runs with other UI tools, and you want to
	///   use DKLib together, you can create proxy window with created from other UI
	///   tools, and you can create and attach screen into that window.
	///
	///   On some platforms (macOS, iOS includes) a window object should be created
	///   on main thread.
	///
	///   On iOS, calling Terminate() will terminate current process and will not
	///   escape into entry function(main). But other desktop OSes does not.
	///
	/// @note
	///   You can set iOS/OSX application delegate by setting SystemConfig.
	///   - For iOS:\n
	///       DKPropertySet::SystemConfig().SetValue("UIApplicationDelegate", "MyAppDelegate");
	///   - For macOS:\n
	///       DKPropertySet::SystemConfig().SetValue("NSApplicationDelegate", "MyAppDelegate");
	///   - For both:\n
	///       DKPropertySet::SystemConfig().SetValue("AppDelegate", "MyAppDelegate"); \n
	///   these properties must be set before calling DKApplication::Run().
	class DKGL_API DKApplication
	{
	public:
		/// environment variable path
		enum class SystemPath 
		{
			SystemRoot,			///< system root. (boot volume on Windows)
			AppRoot,			///< root directory of executable.
			AppResource,		///< application resource directory.
			AppExecutable,		///< directory path where executable is.
			AppData,			///< application's data directory.
			UserHome,			///< home directory path for current user.
			UserDocuments,		///< user's document directory.
			UserPreferences,	///< user's preferences(config) directory.
			UserCache,			///< user's cache directory.
			UserTemp,			///< temporary directory for current user.
		};
		enum class ProcessInfo
		{
			HostName,
			OsName,
			UserName,
			ModulePath,	///< retrieve current module (or executable) path.
		};

		DKApplication();
		DKApplication(int argc, char* argv[]);
		virtual ~DKApplication();

		/// create and running application's main event loop.
		/// only one instance can enter main loop.
		int Run();

		/// Terminate main loop.
		void Terminate(int exitCode);

		/// get DKApplication instance which is running.
		static DKApplication* Instance();

		/// get application main event loop.
		DKEventLoop* EventLoop();

		/// retrieve pre-defined paths
		DKString DefaultPath(SystemPath);
		DKString ProcessInfoString(ProcessInfo);

		/// LoadResource loads application resource into read,writable buffer.
		DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		/// LoadStaticResource loads application resource into read-only buffer. (faster)
		DKObject<DKData> LoadStaticResource(const DKString& res);

		/// get entire display bounds. (displayId = 0 is main screen)
		DKRect DisplayBounds(int displayId) const;
		/// get displayable content bounds. (avaialbe for window frame)
		/// On iOS, android, you can create window with this rect for full-screen app.
		DKRect ScreenContentBounds(int displayId) const;

	protected:
		virtual void OnHidden();        ///< application become hidden. (not minimized)
		virtual void OnRestore();       ///< application has restored.
		virtual void OnActivated();     ///< application has activated.
		virtual void OnDeactivated();   ///< application has deactivated.
		virtual void OnInitialize();    ///< application being initialized. (entering EventLoop)
		virtual void OnTerminate();     ///< application being terminated. (exiting EventLoop)

	private:
		void Initialize();
		void Finalize();
		DKDateTime			initializedAt;
		DKMutex				mutex;
		int					exitCode;
		DKApplicationInterface*	impl;
		friend class DKApplicationInterface;
	};
}
