#include <string>
#include "Config.h"

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

using namespace std;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Common
			{
				void PrintConInfo(wstring message, bool force)
				{
					if (force || Config::Config::Debug)
					{
						ConPrint(L"[Board] " + message + L"\r\n");
					}
				}

				bool DebugPrintEnabled()
				{
					return Config::Config::Debug;
				}
			}
		}
	}
}
