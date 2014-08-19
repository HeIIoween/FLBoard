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
				bool Worker::stopping = false;
				CRITICAL_SECTION Worker::staticOptLock;
				Worker::Instances Worker::instances;

				void Worker::Start(uint threads)
				{
					if (!inited)
					{
						inited = true;

						InitializeCriticalSection(&staticOptLock);
					}

					stopping = false;

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

					if (stopping)
					{
						return;
					}

					stopping = true;

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

					if (stopping)
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
					string errMsg = "";
					
					ThreadData* data = (ThreadData*)threadData;

					while (true)
					{
						(*data).Busy = false;

						// Guard thread will always run
						// If it's not a Guarder, make it run when needed
						WaitForSingleObject(
							(*data).Closing ? (*data).CloseEvent : (*data).WaitEvent,
							(!(*data).Guard || (*data).Closing ? INFINITE : MAX_THREAD_RUN_DELAY)
						);
						
						(*data).Busy = true;

						// Double check the close, because the thread may get it before or after task
						if (!(*data).Keep)
						{
							Sleep(150);

							// Thread be like: bye bye
							break;
						}

						if (!(*data).Closing)
						{
							Sync::APIResponsePackages packages;

							Sync::Client::Run(&packages);

							// Try handle data no matter what Sycn runners says
							if (packages.size() > 0)
							{
								try
								{
									Sync::Listener::Run(packages, (*data).Closing);
								}
								catch (exception &e)
								{
									errMsg = "[Board] At least one Response Task callback rans and failed: ";
									errMsg.append(e.what());

									AddLog(errMsg.c_str());
								}
							}
						}

						Sleep(100); // Take a nap
					}

					// Wait for CPU
					Sleep(200);

					// Free the thread handles
					CloseHandle((*data).WaitEvent);
					CloseHandle((*data).CloseEvent);
					CloseHandle((*data).Thread);

					delete data;

					_endthreadex(0);

					return 0;
				}

				Worker::Worker(uint threads)
				{
					killing = false;
					uint threadOpenLoop;
					multiThreads = false;

					InitializeCriticalSection(&instanceOptLock);

					EnterCriticalSection(&instanceOptLock);

					for (threadOpenLoop = 0; threadOpenLoop < threads; threadOpenLoop++)
					{
						ThreadData* threadData = new ThreadData;

						if (openedThreads.size() == 0)
						{
							(*threadData).Guard = true;
						}
						else
						{
							(*threadData).Guard = false;
						}
						
						(*threadData).Keep = true;
						(*threadData).Busy = false;
						(*threadData).Closing = false;

						(*threadData).WaitEvent = CreateEvent(NULL, true, false, NULL);
						(*threadData).CloseEvent = CreateEvent(NULL, true, false, NULL);
						(*threadData).Thread = (HANDLE)_beginthreadex(NULL, 0, &Thread, threadData, 0, NULL);

						openedThreads.push_back(threadData);

						printError(L"New thread created");
					}

					if (openedThreads.size() > 0)
					{
						multiThreads = true;
					}

					LeaveCriticalSection(&instanceOptLock);
				}

				Worker::~Worker() {
					killing = true;

					EnterCriticalSection(&instanceOptLock);

					printError(L"Closing thread. It may take few minutes");

					while (openedThreads.size() > 0)
					{
						ThreadData* threadData = openedThreads.back();

						openedThreads.pop_back();

						// Only close thread when it's not busy
						if ((*threadData).Busy || !(*threadData).Closing)
						{
							(*threadData).Closing = true;

							// Bring it back
							SetEvent((*threadData).WaitEvent);

							openedThreads.push_front(threadData);

							Sleep(100);

							continue;
						}

						(*threadData).Keep = false;

						if (!SetEvent((*threadData).CloseEvent))
						{
							printError(L"Try sending shutdown to thread, but failed");

							continue;
						}
						else
						{
							printError(L"Shutdown signal has sent");
						}

						switch (WaitForSingleObject((*threadData).Thread, MAX_THREAD_CLOSE_WAITING_TIME))
						{
						case WAIT_OBJECT_0:
							printError(L"Thread closed");
							break;

						default:
							printError(L"Thread not closed as excepted");
							break;
						}
					}

					LeaveCriticalSection(&instanceOptLock);

					DeleteCriticalSection(&instanceOptLock);

					printError(L"Thread data has been cleared");
				};

				void Worker::wakeUp()
				{
					bool waked = false;
					uint threadSize = 0;
					
					// If we are shuting down, no new wake up request
					if (killing)
					{
						return;
					}

					EnterCriticalSection(&instanceOptLock);

					if (multiThreads) // If we can have not only the guard thread, so we can select another one to enable
					{
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