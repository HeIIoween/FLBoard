#include <string>
#include <map>

#include "Parameter.h"

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
				// Data container for event sending
				typedef map <string, wstring> EventData;

				// Shadow of Sync::EventCallback
				typedef void(*EventCallback)(wstring source, Data::Parameter parameter);

				// Add a new event listener
				EXPORT void Listen(string eventName, EventCallback callback);

				// Request data sync to available APIs acordding to setting
				EXPORT void Send(string type, EventData data);

				// Import another API plugin's configuration
				EXPORT void Import(string scPluginCfgFile, bool emptyFail = true);

				// Get all working API servers
				EXPORT Data::ParameterSet APIs();
			}
		}
	}
}
