#include <string>
#include <map>

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

using namespace std;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Event
			{
				typedef map <string, wstring> EventParameter;

				typedef map <string, wstring> EventData;

				// Shadow of Sync::EventCallback
				typedef void(*EventCallback)(wstring source, EventParameter parameter);

				// Too many details you say?
				EXPORT void Listen(string eventName, EventCallback callback);
				EXPORT void Send(string type, EventData data);
			}
		}
	}
}
