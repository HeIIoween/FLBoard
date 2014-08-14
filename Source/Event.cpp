#include "Event.h"
#include "Misc.h"
#include "Sync.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Event
			{
				EXPORT void Listen(string eventName, EventCallback callback)
				{
					Sync::Listener::Listen(eventName, callback);
				}

				EXPORT void Send(string type, EventData data)
				{
					EventData::iterator dIter;

					Sync::DataValue value;
					Sync::DataItem item;

					for (dIter = data.begin(); dIter != data.end(); dIter++)
					{
						value[dIter->first] = dIter->second;
					}

					item[type] = value;

					Sync::Client::Send(item);
				}
			}
		}
	}
}
