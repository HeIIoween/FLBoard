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
				// Parameter for event callback
				typedef map <wstring, wstring> EventParameter;

				// Data container for event sending
				typedef map <string, wstring> EventData;

				// Shadow of Sync::EventCallback
				typedef void(*EventCallback)(wstring source, EventParameter parameter);

				// Add a new event listener
				EXPORT void Listen(string eventName, EventCallback callback);

				// Request data sync to available APIs acordding to setting
				EXPORT void Send(string type, EventData data);
			}
		}
	}
}
