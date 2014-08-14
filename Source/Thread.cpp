#include <windows.h>
#include <time.h>
#include <process.h>

#include "Common.h"
#include "Thread.h"
#include "Sync.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Thread
			{
				Worker* Worker::_instance = 0;
				bool Worker::_shutdown = false;

				Worker *Worker::Start()
				{
					if (0 == _instance) {
						_instance = new Worker();
					}

					return _instance;
				}

				void Worker::Stop()
				{
					if (0 != _instance) {
						delete _instance;

						_instance = 0;
					}
				}

				Worker::Worker()
				{
					_shutdown = false;

					this->thread = (HANDLE)_beginthreadex(0, 0, &Worker::Thread, NULL, 0, NULL);

					CloseHandle(this->thread);
				}

				Worker::~Worker()
				{
					Common::PrintConInfo(L"freeing Thread...");

					_shutdown = true;
					WaitForSingleObject(&Worker::Thread, INFINITE);

					Common::PrintConInfo(L"Thread freed...");
				}
				
				unsigned __stdcall Worker::Thread(void *args)
				{
					int sleep = 1,
						lastDelay = sleep, currentDelay = sleep;
					
					bool failedLoop = false;

					clock_t startTime, endTime;

					while (!_shutdown)
					{
						startTime = clock();

						Sync::APIResponsePackage package;

						if (!Sync::Client::Run(&package))
						{
							failedLoop = true;
						}
						else
						{
							Sync::Listener::Run(package);

							failedLoop = false;
						}

						endTime = clock();

						currentDelay = (endTime - startTime) / (CLOCKS_PER_SEC / 1000);

						if (!failedLoop)
						{
							sleep = (currentDelay + lastDelay) / 2;

							lastDelay = currentDelay;
						}
						else
						{
							sleep *= 2;

							lastDelay = sleep;
						}

						if (sleep > MAX_THREAD_WAITING_TIME)
						{
							sleep = MAX_THREAD_WAITING_TIME;
						}

						if (sleep < 1)
						{
							sleep = 1;
						}

						Sleep(sleep * 1000); // One second by force 
					}

					_endthreadex(0);
					return 0;
				}
			}
		}
	}
}