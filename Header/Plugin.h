#include "../../flhookplugin_sdk/headers/FLHook.h"
#include "../../flhookplugin_sdk/headers/plugin.h"

#ifndef PLUGIN_FILE_INCLUDED
	#define PLUGIN_FILE_INCLUDED

	extern PLUGIN_RETURNCODE returncode;
#endif

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode();
EXPORT PLUGIN_INFO* Get_PluginInfo();