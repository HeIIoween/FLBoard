#include "Plugin.h"
#include "Hooks.h"
#include "Event.h"

using namespace raincious::FLHookPlugin::Board;

PLUGIN_RETURNCODE returncode;

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();

	p_PI->sName = "Board Plugin by NeXoSE <nexose@gmail.com>";
	p_PI->sShortName = "board";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;

	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));

	return p_PI;
}