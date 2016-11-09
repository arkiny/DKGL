//
//  File: DKApplication.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKRect.h"

////////////////////////////////////////////////////////////////////////////////
// DKApplication
// This is application entry point class.
// You can use this class for creating application one-sourced, cross-platform.
// You need subclass of this class to override application behaviors.
// You can have multiple instances of this class, but only one instance can run,
// other instances will be blocked until previous instance has finished run.
// This class provides retrieving process environments information.
//
// Subclass note:
// 1. Override OnInitialize(). this is called when application to be
//    initialized. You may need create single or multiple window(s)
//    (as DKWindow), and screen (DKScreen) objects to attached into a window.
//    One screen instance per one. But this is optional.
//    You don't have to create windows, screens if your application does not
//    need window to display. (ie console application).
//    Whether your application has window or not, an application will have
//    main loop (DKEventLoop) and looping till Terminate() has called.
// 2. Override OnTerminate(). this is called when application terminated by
//    calling Terminate(). Your cleanup operations must be here.
//    (destory windows, destroy screens, closing files, etc.)
//
// Note:
//   Using this class is optional. You can initialize window, screen at any
//   time. If your application runs with other UI tools, and you want to
//   use DKLib together, you can create proxy window with created from other UI
//   tools, and you can create and attach screen into that window.
//
//   On some platforms (OS X, iOS includes) a window object should be created
//   on main thread.
//
//   On iOS, calling Terminate() will terminate current process and will not
//   escape into entry function(main). But other desktop OSes does not.
//
//   You can set iOS/OSX application delegate by setting SystemConfig.
//   For iOS:
//    DKPropertySet::DefaultConfig().SetValue("UIApplicationDelegate", "MyAppDelegate");
//   For OS X:
//    DKPropertySet::DefaultConfig().SetValue("NSApplicationDelegate", "MyAppDelegate");
//   these properties must be set before calling DKApplication::Run().
////////////////////////////////////////////////////////////////////////////////

#ifdef GetUserName
#undef GetUserName
#endif

namespace DKFramework
{
	class DKApplicationInterface;
	class DKGL_API DKApplication
	{
		friend class DKApplicationInterface;

	public:
		enum SystemPath
		{
			SystemPathSystemRoot = 0,  // system root. (boot volume on Windows)
			SystemPathAppRoot,         // root directory of executable.
			SystemPathAppResource,     // application resource directory.
			SystemPathAppExecutable,   // directory path where executable is.
			SystemPathAppData,         // application's data directory.
			SystemPathUserHome,        // home directory path for current user.
			SystemPathUserDocuments,   // user's document directory.
			SystemPathUserPreferences, // user's preferences(config) directory.
			SystemPathUserCache,       // user's cache directory.
			SystemPathUserTemp,        // temporary directory for current user.
		};
		enum EnvironmentVariable
		{
			EnvHostName,
			EnvOsName,
			EnvUserName,
			EnvModulePath,	// retrieve current module (or executable) path.
		};

		DKApplication(void);
		DKApplication(int argc, char* argv[]);
		virtual ~DKApplication(void);

		void SetArgs(int argc, char* argv[]);
		const DKArray<DKString>& Args(void) const { return args; }

		// create and running application's main event loop.
		// only one instance can enter main loop.
		int Run();

		// Terminate main loop.
		void Terminate(int exitCode);

		// get DKApplication instance which is running.
		static DKApplication* Instance(void);

		// get application main event loop.
		DKEventLoop* EventLoop(void);

		// retrieve pre-defined paths
		DKString EnvironmentPath(SystemPath);
		DKString EnvironmentString(EnvironmentVariable);

		// LoadResource loads application resource into read,writable buffer.
		DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc = DKAllocator::DefaultAllocator());
		// LoadStaticResource loads application resource into read-only buffer. (faster)
		DKObject<DKData> LoadStaticResource(const DKString& res);

		// get entire display bounds. (displayId = 0 is main screen)
		DKRect DisplayBounds(int displayId) const;
		// get displayable content bounds. (avaialbe for window frame)
		// On iOS, android, you can create window with this rect for full-screen app.
		DKRect ScreenContentBounds(int displayId) const;

	protected:
		virtual void OnHidden(void);        // application become hidden. (not minimized)
		virtual void OnRestore(void);       // application has restored.
		virtual void OnActivated(void);     // application has activated.
		virtual void OnDeactivated(void);   // application has deactivated.
		virtual void OnInitialize(void);    // application being initialized. (entering EventLoop)
		virtual void OnTerminate(void);     // application being terminated. (exiting EventLoop)

	private:
		void Initialize();
		void Finalize();
		DKDateTime			initializedAt;
		DKArray<DKString>	args;
		DKMutex				mutex;
		int					exitCode;
		DKApplicationInterface*	impl;
	};
}
