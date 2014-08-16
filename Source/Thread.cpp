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
					bool failedLoop = false;

					double startTime = 0, endTime = 0;
					
					ThreadData* data = (ThreadData*)threadData;
					
					while (true)
					{
						WaitForSingleObject((*data).WaitEvent, INFINITE);

						if ((*data).Close)
						{
							// Thread be like: bye bye
							break;
						}

						Sync::APIResponsePackages packages;

						if (!Sync::Client::Run(&packages))
						{
							failedLoop = true;
						}
						else
						{
							failedLoop = false;

							try
							{
								Sync::Listener::Run(packages);
							}
							catch (...)
							{
								AddLog("[Board] At least one Response Task callback rans and failed.");
							}
						}
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

						// CloseHandle(threadHandle);

						openedThreads.push_back(threadData);

						printError(L"New thread created");

						Sleep(100);
					}

					LeaveCriticalSection(&instanceOptLock);
				}

				Worker::~Worker() {
					EnterCriticalSection(&instanceOptLock);
					
					ThreadDatas::iterator iter;

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

						Sleep(100);
					}

					LeaveCriticalSection(&instanceOptLock);

					DeleteCriticalSection(&instanceOptLock);

					printError(L"Thread data has been cleared");
				};

				void Worker::wakeUp()
				{
					EnterCriticalSection(&instanceOptLock);

					ThreadDatas::iterator iter;

					for (iter = openedThreads.begin(); iter != openedThreads.end(); iter++)
					{
						SetEvent((*iter)->WaitEvent);
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