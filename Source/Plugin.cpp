#include "Plugin.h"
#include "Hooks.h"

PLUGIN_RETURNCODE returncode;

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();

	p_PI->sName = "API Board Plugin by NeXoSE <nexose@gmail.com>";
	p_PI->sShortName = "board";
	p_PI->bMayPause = false; // Cannot be pause as it has its own thread manager
	p_PI->bMayUnload = false; // Don't unload, we got handles in control
	p_PI->ePluginReturnCode = &returncode;

	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIServerImpl::Shutdown, PLUGIN_HkIServerImpl_Shutdown, 0));

	return p_PI;
}