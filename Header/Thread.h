#include <vector>

#include "../../flhookplugin_sdk/headers/FLHook.h"

#define MAX_THREAD_CLOSE_WAITING_TIME 15000
#define MAX_THREAD_RUN_DELAY 5000

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
							bool Guard;
							bool Keep;
							bool Busy;
							HANDLE WaitEvent;
							HANDLE Thread;
						} threadData;

						typedef vector <ThreadData*> ThreadDatas;

						ThreadDatas openedThreads;
						bool multiThreads;

						bool killing;

						Worker(uint threads);
						~Worker();

						void wakeUp();

						void printError(wstring error);
				};
			}
		}
	}
}