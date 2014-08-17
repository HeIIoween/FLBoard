#include "Plugin.h"
#include "Hooks.h"
#include "Config.h"
#include "Thread.h"
#include "Common.h"
#include "Event.h"
#include "Main.h"

using namespace std;

namespace HkIServerImpl
{
	void __stdcall Shutdown()
	{
		Main::PluginShutdown();
	}
}

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;

	static bool lock = false;
	string loadError = "";

	// Anit rehash crash lock
	if (lock)
	{
		return;
	}

	lock = true;

	try
	{
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);

		string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\board.cfg";

		Main::PluginShutdown();

		Config::Config::Debug = IniGetB(scPluginCfgFile, "General", "Debug", false);
		Config::Config::Threads = IniGetI(scPluginCfgFile, "General", "Worker", 1);

		if (Config::Config::Threads <= 0)
		{
			Common::PrintConInfo(L"Are you aure? The \"Worker\" setting in your config file is too small for working.", true);
		}

		// Clear old API info
		Config::Config::APIs.clear();
		Event::Import(scPluginCfgFile, false);

		Thread::Worker::Start(Config::Config::Threads);
	}
	catch (exception& e)
	{
		loadError = e.what();

		Common::PrintConInfo(L"[!!!] Can't load setting on error: " + wstring(loadError.begin(), loadError.end()));

		try
		{
			Main::PluginShutdown();
		} 
		catch (exception& e2)
		{
			loadError = e2.what();

			Common::PrintConInfo(L"[!!!] ... also we got a error when try to shut plugin down: " + wstring(loadError.begin(), loadError.end()));
		}
	}

	lock = false;
}