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
					
					while (true)
					{
						// Double check the close, because the thread may get it before or after task
						if ((*data).Close)
						{
							break;
						}

						if ((*data).Retrying > 0)
						{
							(*data).Busy = false;

							WaitForSingleObject((*data).WaitEvent, INFINITE);

							(*data).Busy = true;
						}

						if ((*data).Close)
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
							(*data).Retrying = 0;

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

						(*threadData).Close = false;
						(*threadData).WaitEvent = CreateEvent(NULL, true, false, NULL);
						(*threadData).Thread = (HANDLE)_beginthreadex(0, 0, &Thread, threadData, 0, NULL);

						openedThreads.push_back(threadData);

						printError(L"New thread created");

						Sleep(100);
					}

					LeaveCriticalSection(&instanceOptLock);
				}

				Worker::~Worker() {
					EnterCriticalSection(&instanceOptLock);
					
					ThreadDatas::iterator iter;

					printError(L"Closing thread. It may take few minutes");

					for (iter = openedThreads.begin(); iter != openedThreads.end(); iter++)
					{
						(*iter)->Close = true;

						if (!SetEvent((*iter)->WaitEvent))
						{
							printError(L"Try sending shutdown to thread, but failed");

							// Do a close here too
							CloseHandle((*iter)->WaitEvent);

							continue;
						}

						CloseHandle((*iter)->WaitEvent);

						switch (WaitForSingleObject((*iter)->Thread, MAX_THREAD_CLOSE_WAITING_TIME))
						{
						case WAIT_OBJECT_0:
							printError(L"Thread closed");
							break;

						default:
							printError(L"Encounter a problem when try to closing thread");
							break;
						}

						// Free the thread handle
						CloseHandle((*iter)->Thread);

						delete (*iter); // Delete the container it self
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