#include "Plugin.h"
#include "Hooks.h"
#include "Thread.h"
#include "Config.h"
#include "Common.h"
#include "Event.h"
#include "Misc.h"

using namespace std;
using namespace raincious::FLHookPlugin::Board;

bool bGlobalDebug;

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\board.cfg";

	Config::Config::Debug = IniGetB(scPluginCfgFile, "General", "Debug", false);

	Event::Import(scPluginCfgFile, false);

	Thread::Worker::Start();
}