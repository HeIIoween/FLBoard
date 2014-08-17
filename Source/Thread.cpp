#include <windows.h>
#include <process.h>

#include "Common.h"
#include "Thread.h"
#include "Misc.h"
#include "Sync.h"

#include "../../flhookplugin_sdk/headers/FLHook.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Thread
			{
				bool Worker::inited = false;
				CRITICAL_SECTION Worker::staticOptLock;
				Worker::Instances Worker::instances;

				void Worker::Start(uint threads)
				{
					if (!inited)
					{
						inited = true;

						InitializeCriticalSection(&staticOptLock);
					}

					EnterCriticalSection(&staticOptLock);

					Worker* worker = new Worker(threads);

					instances.push_back(worker);

					LeaveCriticalSection(&staticOptLock);
				}

				void Worker::Stop()
				{
					Instances::iterator iter;

					if (!inited)
					{
						return;
					}

					EnterCriticalSection(&staticOptLock);

					for (iter = instances.begin(); iter != instances.end(); iter++)
					{
						delete (*iter);
					}

					instances.clear();

					LeaveCriticalSection(&staticOptLock);
					DeleteCriticalSection(&staticOptLock);

					inited = false;
				}

				void Worker::Activate()
				{
					Instances::iterator iter;

					if (!inited)
					{
						return;
					}

					EnterCriticalSection(&staticOptLock);

					for (iter = instances.begin(); iter != instances.end(); iter++)
					{
						(*iter)->wakeUp();
					}

					LeaveCriticalSection(&staticOptLock);
				}

				unsigned __stdcall Worker::Thread(void *threadData)
				{
					double startTime = 0, endTime = 0;
					uint nextSleep = 1;
					string errMsg = "";
					
					ThreadData* data = (ThreadData*)threadData;
					
					while ((*data).Keep)
					{
						if ((*data).Retrying > 0)
						{
							(*data).Busy = false;

							WaitForSingleObject((*data).WaitEvent, INFINITE);

							(*data).Busy = true;
						}

						// Double check the close, because the thread may get it before or after task
						if (!(*data).Keep)
						{
							// Thread be like: bye bye
							break;
						}

						Sync::APIResponsePackages packages;

						if (!Sync::Client::Run(&packages))
						{
							if ((*data).Retrying < MAX_THREAD_MAX_RETRY)
							{
								nextSleep = ++(*data).Retrying;

								if (nextSleep > MAX_THREAD_RETRY_DELAY)
								{
									nextSleep = MAX_THREAD_RETRY_DELAY;
								}
							}
							else
							{
								// Keep trying, but as Busy flaged as false
								(*data).Busy = false;
							}

							Sleep(1000 * nextSleep);
						}
						else
						{
							nextSleep = 1;
							(*data).Retrying = 0;
						}

						// Try handle data no matter what Sycn runners says
						if (packages.size() > 0)
						{
							try
							{
								Sync::Listener::Run(packages);
							}
							catch (exception &e)
							{
								errMsg = "[Board] At least one Response Task callback rans and failed: ";
								errMsg.append(e.what());

								AddLog(errMsg.c_str());
							}
						}

						Sleep(100);
					}

					_endthreadex(0);

					return 0;
				}

				Worker::Worker(uint threads)
				{
					uint threadOpenLoop;

					InitializeCriticalSection(&instanceOptLock);

					EnterCriticalSection(&instanceOptLock);

					for (threadOpenLoop = 0; threadOpenLoop < threads; threadOpenLoop++)
					{
						ThreadData* threadData = new ThreadData;

						(*threadData).Keep = true;
						(*threadData).WaitEvent = CreateEvent(NULL, true, false, NULL);
						(*threadData).Thread = (HANDLE)_beginthreadex(0, 0, &Thread, threadData, 0, NULL);

						openedThreads.push_back(threadData);

						printError(L"New thread created");
					}

					LeaveCriticalSection(&instanceOptLock);
				}

				Worker::~Worker() {
					EnterCriticalSection(&instanceOptLock);
					
					printError(L"Closing thread. It may take few minutes");

					while (openedThreads.size() > 0)
					{
						ThreadData* threadData = openedThreads.back();
						openedThreads.pop_back();

						threadData->Keep = false;

						if (!SetEvent(threadData->WaitEvent))
						{
							printError(L"Try sending shutdown to thread, but failed");

							// Do a close here too
							CloseHandle(threadData->WaitEvent);

							continue;
						}

						switch (WaitForSingleObject(threadData->Thread, MAX_THREAD_CLOSE_WAITING_TIME))
						{
						case WAIT_OBJECT_0:
							printError(L"Thread closed");
							break;

						default:
							printError(L"Encountered a problem when trying to close thread");
							break;
						}

						// Free the thread handles
						CloseHandle(threadData->WaitEvent);
						CloseHandle(threadData->Thread);

						delete (threadData); // Delete the data container it self
					}

					LeaveCriticalSection(&instanceOptLock);

					DeleteCriticalSection(&instanceOptLock);

					printError(L"Thread data has been cleared");
				};

				void Worker::wakeUp()
				{
					bool waked = false;

					EnterCriticalSection(&instanceOptLock);

					ThreadDatas::iterator iter, lastIter;

					for (iter = openedThreads.begin(); iter != openedThreads.end(); iter++)
					{
						if (!(*iter)->Busy)
						{
							SetEvent((*iter)->WaitEvent);
							waked = true;
							break;
						}
						else
						{
							lastIter = iter;
						}
					}

					if (!waked)
					{
						SetEvent((*lastIter)->WaitEvent);
					}

					LeaveCriticalSection(&instanceOptLock);
				}

				void Worker::printError(wstring error)
				{
					if (!Common::DebugPrintEnabled())
					{
						return;
					}

					wstring message = L"[Thread] " + error + L" <";

					message.append(L"Threads: ");
					message.append(Misc::Encode::stringToWstring(itos(openedThreads.size())));

					message.append(L">");

					Common::PrintConInfo(message);
				}
			}
		}
	}
}