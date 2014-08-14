#include <string>

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
				void PrintConInfo(wstring message)
				{
					ConPrint(L"[Board] " + message + L"\r\n");
				}
			}
		}
	}
}
