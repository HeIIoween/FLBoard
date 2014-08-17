// Include system needed 
#include <windows.h>

#include "Thread.h"
#include "Sync.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Main
			{
				void PluginShutdown()
				{
					// Runing as follow order
					Thread::Worker::Stop(); // 1, Everythings finer with stopped threads

					Sync::Client::Release(); // 2, Clear all sync info
				}
			}
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}