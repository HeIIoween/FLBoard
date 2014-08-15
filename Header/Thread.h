#define MAX_THREAD_WAITING_TIME 30

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
						static Worker* Start();
						static void Stop();

					protected:
						static Worker* _instance;
						static bool _shutdown;
						
						HANDLE thread;

						Worker();
						~Worker();

						static unsigned __stdcall Thread(void *args);
				};
			}
		}
	}
}