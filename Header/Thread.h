#include <vector>

#include "../../flhookplugin_sdk/headers/FLHook.h"

#define MAX_THREAD_CLOSE_WAITING_TIME 60000
#define MAX_THREAD_RETRY_DELAY 10
#define MAX_THREAD_MAX_RETRY 10

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Thread
			{
				class Worker
				{
					public:
						static void Start(uint threads);
						static void Stop();
						static void Activate();

					protected:
						typedef vector <Worker*> Instances;

						static bool inited;

						static CRITICAL_SECTION staticOptLock;

						static unsigned __stdcall Thread(void *args);

						static Instances instances;

						CRITICAL_SECTION instanceOptLock;

						typedef struct ThreadData
						{
							bool Close;
							bool Busy;
							uint Retrying;
							HANDLE WaitEvent;
							HANDLE Thread;
						} threadData;

						typedef vector <ThreadData*> ThreadDatas;

						ThreadDatas openedThreads;

						Worker(uint threads);
						~Worker();

						void wakeUp();

						void printError(wstring error);
				};
			}
		}
	}
}