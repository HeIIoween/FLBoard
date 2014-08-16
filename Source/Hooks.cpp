#include "Plugin.h"
#include "Hooks.h"
#include "Config.h"
#include "Thread.h"
#include "Common.h"
#include "Event.h"
#include "Main.h"

using namespace std;

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;
	static bool inited = false;

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	uint openedThreads = 0;

	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\board.cfg";

	// Will stop sync and thread
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

	if (!inited)
	{
		inited = true;

		atexit(Main::PluginShutdown);
	}
}