#include "Sync.h"
#include "Misc.h"
#include "Common.h"

using namespace Misc;
using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Sync
			{
				// Client
				Client::Instances Client::instances;

				Client* Client::Get(APILogin clientLogin)
				{
					static bool inited = false;

					if (!inited)
					{
						inited = true;

						atexit(Release);
					}

					Client* client = new Client(clientLogin);

					instances.push_back(client);

					return client;
				}

				void Client::Send(DataItem data)
				{
					Instances::iterator it;

					if (instances.empty())
					{
						return;
					}

					for (it = instances.begin(); it < instances.end(); it++)
					{
						(*it)->addQueue(data);
					}
				}

				void Client::Release()
				{
					Instances::iterator it;

					if (instances.empty())
					{
						return;
					}

					for (it = instances.begin(); it < instances.end(); it++)
					{
						delete *it;
					}
				}

				Client::Client(APILogin client)
				{
					wstring message;
					string loginErrorMessage;
					APIResponseStatus loginResponse;

					loginInfo = client;
					
					verifyer.key(loginInfo.Secret);

					loginResponse = login(loginErrorMessage);

					if (loginResponse != SUCCEED)
					{
						message = L"API Login failed: ";

						switch (loginResponse) {
						case FAILED_STATUS:
							message.append(L"Server error");
							break;

						case INVALID_RESPONSE:
							message.append(L"Server response invalid");
							break;

						case INVALID_TOKEN:
							message.append(L"Server responses invalid token");
							break;

						case INVALID_SECERT:
							message.append(L"Server unidentified");
							break;

						case INVALID_ACK:
							message.append(L"Server failed to acknowledge");
							break;

						case ERROR_MESSAGE:
							message.append(L"Server response a error message: ");
							message.append(wstring(loginErrorMessage.begin(), loginErrorMessage.end()));
							break;
						}

						message.append(L". Player information will not send to server: ");
						message.append(wstring(loginInfo.URI.begin(), loginInfo.URI.end()));
						message.append(L".");

						Common::PrintConInfo(message);
					}
					else
					{
						message.append(L"Connection to ");
						message.append(wstring(loginInfo.URI.begin(), loginInfo.URI.end()));
						message.append(L" has been established.");

						Common::PrintConInfo(message);

						enabled = true;
					}
				}

				Client::~Client()
				{
					logoff();
				}

				void Client::setJsonCommonHeader(Http::HttpHandler& http)
				{
					http.header("Accept: application/json");
					http.header("Content-Type: application/json");
					http.header("charsets: utf-8");
				}

				APIResponseStatus Client::login(string &errorMessage)
				{
					Json::Value loginParameter;
					Json::Value responseRoot;
					Json::Value responseToken;
					Json::Reader responseReader;

					loginParameter["Task"] = "Login";
					loginParameter["Cecret"] = verifyer.gen();
					loginParameter["Account"] = Encode::UTF8Encode(loginInfo.Account);
					loginParameter["Password"] = Encode::UTF8Encode(loginInfo.Password);

					Http::Post http(loginInfo.URI);

					setJsonCommonHeader(http);

					http.data(loginParameter.toStyledString(), "");

					string result = http.result();

					// Server not return the right status
					if (http.status() != 200)
					{
						return FAILED_STATUS;
					}

					// Server returns invalid content format
					if (!responseReader.parse(result, responseRoot))
					{
						return INVALID_RESPONSE;
					}

					if (!verifyer.pair(loginParameter["Cecret"].asInt(), JsonHelper::GetValueInt(responseRoot, "Cecret")))
					{
						return INVALID_SECERT;
					}

					if (JsonHelper::GetValueStr(responseRoot, "Ack") != loginParameter["Task"].asString())
					{
						return INVALID_ACK;
					}

					// Get server error information
					if (JsonHelper::GetValueStr(responseRoot, "Error") != "")
					{
						errorMessage = JsonHelper::GetValueStr(responseRoot, "Error");

						return ERROR_MESSAGE;
					}

					server.Token = JsonHelper::GetValueStr(responseRoot, "Token");

					server.Delay = JsonHelper::GetValueUint(
						responseRoot, "Delay") * 1000; // Server can only return seconds

					server.Name = Encode::UTF8Decode(
						JsonHelper::GetValueStr(responseRoot, "Name")
						);

					server.QueueLimit = JsonHelper::GetValueUint(
						responseRoot, "Queue");

					// API Server not returning valid token
					if (server.Token == "")
					{
						return INVALID_TOKEN;
					}

					// API Server friendly name
					if (server.Name == L"")
					{
						server.Name = L"Untitled ";
						server.Name.append(wstring(loginInfo.URI.begin(), loginInfo.URI.end()));
					}

					// API Server delay time
					if (server.Delay < 0)
					{
						server.Delay = SYNC_CLIENT_MAX_DELAY;
					}
					else
					{
						if (server.Delay > SYNC_CLIENT_MAX_DELAY)
						{
							server.Delay = SYNC_CLIENT_MAX_DELAY;
						}
					}

					// API Server delay time
					if (server.QueueLimit <= 0)
					{
						server.QueueLimit = SYNC_CLIENT_DEFAULT_QUEUE;
					}
					else
					{
						if (server.QueueLimit > SYNC_CLIENT_MAX_QUEUE)
						{
							server.QueueLimit = SYNC_CLIENT_MAX_QUEUE;
						}
					}

					return SUCCEED;
				}

				APIResponseStatus Client::logoff()
				{
					Json::Value logoffParameter;
					Json::Value responseParameter;
					Json::Reader responseReader;

					Http::Post http(loginInfo.URI);

					setJsonCommonHeader(http);

					if (!enabled) 
					{
						return FAILED_LOGIN;
					}

					logoffParameter["Task"] = "Logoff";
					logoffParameter["Cecret"] = verifyer.gen();
					logoffParameter["Token"] = server.Token;

					http.data(logoffParameter.toStyledString(), "");

					string result = http.result();

					if (http.status() != 200)
					{
						return FAILED_STATUS;
					}

					if (!responseReader.parse(result, responseParameter))
					{
						return INVALID_RESPONSE;
					}

					if (!verifyer.pair(logoffParameter["Cecret"].asInt(), JsonHelper::GetValueInt(responseParameter, "Cecret")))
					{
						return INVALID_SECERT;
					}

					if (JsonHelper::GetValueStr(responseParameter, "Ack") != logoffParameter["Task"].asString())
					{
						return INVALID_ACK;
					}

					return SUCCEED;
				}

				APIResponseStatus Client::sync(Json::Value root, APIResponses* responses, bool noRetry)
				{
					Json::Value sendParameter;
					Json::Value responseRoot;
					Json::Value responseToken;
					Json::Reader responseReader;
					string errMessage = "";

					// Check if server is available
					if (!enabled)
					{
						return DISABLED;
					}

					// Check skips
					if (skips > 0)
					{
						skips--;

						return SKIPED;
					}

					sendParameter["Task"] = "Sync";
					sendParameter["Token"] = server.Token;
					sendParameter["Cecret"] = verifyer.gen();
					sendParameter["Data"] = root;

					Http::Post http(loginInfo.URI);

					setJsonCommonHeader(http);

					http.data(sendParameter.toStyledString(), "");

					string result = http.result();
					
					// Catch status error and try handle it when we can
					switch (http.status())
					{
					case 200:
						// Level a break to pass, every case below shall return
						break;

					case 403:
						if (noRetry)
						{
							return FAILED_LOGIN;
						}

						if (login(errMessage) == SUCCEED)
						{
							return sync(root, responses, true);
						}
						else
						{
							skips += 20;
						}
						break;

					default:
						skips += 5;

						return FAILED_STATUS;
					}

					// Parse response
					if (!responseReader.parse(result, responseRoot))
					{
						return INVALID_RESPONSE;
					}

					if (!responseRoot.isObject())
					{
						return INVALID_RESPONSE;
					}

					if (!verifyer.pair(sendParameter["Cecret"].asInt(), JsonHelper::GetValueInt(responseRoot, "Cecret")))
					{
						return INVALID_SECERT;
					}

					if (JsonHelper::GetValueStr(responseRoot, "Ack") != sendParameter["Task"].asString())
					{
						return INVALID_ACK;
					}

					if (!responseRoot["Tasks"].isArray())
					{
						skips += 10;

						return INVALID_DATA;
					}

					size_t taskSize = responseRoot["Tasks"].size();
					for (size_t taskLoop = 0; taskLoop < taskSize; taskLoop++)
					{
						APIResponseTask responseTask;
						string responseType;
						Json::Value responseData;

						if (!responseRoot["Tasks"][taskLoop].isObject())
						{
							continue;
						}

						responseType = JsonHelper::GetValueStr(responseRoot["Tasks"][taskLoop], "Type");

						if (responseType == "")
						{
							continue;
						}

						responseData = responseRoot["Tasks"][taskLoop]["Data"];

						if (!responseData.isObject())
						{
							continue;
						}

						vector <string> dataKeys = responseData.getMemberNames();
						vector <string>::iterator dataIter;

						for (dataIter = dataKeys.begin(); dataIter != dataKeys.end(); dataIter++)
						{
							string dataKey = dataIter->c_str();

							responseTask.Data[dataKey] =
								Encode::UTF8Decode(JsonHelper::GetValueStr(responseData, dataIter->c_str()));
						}

						responseTask.Type = responseType;

						(*responses).push_back(responseTask);
					}

					return SUCCEED;
				}

				void Client::addQueue(DataItem data)
				{
					uint qLength = 0;
					DataItem item;

					InitializeCriticalSection(&queueSycLock);

					qLength = sendingQueue.size();

					if (qLength > server.QueueLimit)
					{
						sendingQueue.pop();
					}

					sendingQueue.push(data);

					DeleteCriticalSection(&queueSycLock);
				}

				APIResponseStatus Client::sendQueue(APIResponses* responses)
				{
					Json::Value items = Json::Value(Json::arrayValue);
					DataItem queueItem;
					clock_t currentTime = clock();
					Queue opearatingQueue, emptyQueue;

					if ((ulong)currentTime < (ulong)server.lastSent + server.Delay)
					{
						return PERIOD_LIMIT;
					}

					server.lastSent = currentTime;

					// OK, let me copy this queue so we will get rid of lock problem
					InitializeCriticalSection(&queueSycLock);

					if (!sendingQueue.empty())
					{
						opearatingQueue.swap(sendingQueue);
						// Now the empty opearatingQueue becomes new sendingQueue
						// And the sendingQueue becomes new opearatingQueue with datas
					}

					// Unlock, the queue are free
					DeleteCriticalSection(&queueSycLock);

					// Loop all queues until it empty
					while (!opearatingQueue.empty())
					{
						queueItem = opearatingQueue.front();

						DataItem::iterator itemIterator;
						DataValue::iterator valueIterator;
						
						Json::Value item;

						for (itemIterator = queueItem.begin(); itemIterator != queueItem.end(); itemIterator++)
						{
							Json::Value value;
							for (valueIterator = itemIterator->second.begin(); valueIterator != itemIterator->second.end(); valueIterator++)
							{
								value[Encode::UTF8Encode(valueIterator->first)] =
									Encode::UTF8Encode(valueIterator->second);
							}
							item[Encode::UTF8Encode(itemIterator->first)] = value;
						}
						items.append(item);

						opearatingQueue.pop();
					}

					if (items.empty())
					{
						return NO_TASK;
					}

					return sync(items, responses, false);
				}

				bool Client::Run(APIResponsePackage* package)
				{
					Instances::iterator instanceIter;
					bool succeed = true;
					
					for (instanceIter = instances.begin(); instanceIter != instances.end(); ++instanceIter) {
						APIResponses responses;

						APIResponseStatus status = (*instanceIter)->sendQueue(&responses);

						switch (status)
						{
						case SUCCEED:

						case PERIOD_LIMIT:
							// Consider the server's own PERIOD_LIMIT as success
							// as it's not the reason so slow the thread

						case INVALID_DATA:
							// Consider this as the API application error
							// No need to slow the thread
							break;

						default:
							succeed = false;
							break;
						}

						(*package).API = (*instanceIter)->server.Name;
						(*package).Responses = responses;
					}

					return succeed;
				}

				// Listener
				Listener::Events Listener::events;

				void Listener::Listen(string eventName, EventCallback eventCallback)
				{
					events[eventName].Handlers.push_back(eventCallback);

					return;
				}

				void Listener::Run(Sync::APIResponsePackage &package)
				{
					APIResponses::iterator responsesIter;

					for (responsesIter = package.Responses.begin(); responsesIter != package.Responses.end(); responsesIter++)
					{
						trigger(package.API, responsesIter->Type, responsesIter->Data);
					}

					return;
				}

				void Listener::trigger(wstring source, string eventName, ResponseValue response)
				{
					Events::iterator eventIter = events.find(eventName);

					if (eventIter == events.end()) {
						return;
					}

					EventHandlers::iterator eHandlerIter;
					for (eHandlerIter = events[eventName].Handlers.begin(); eHandlerIter != events[eventName].Handlers.end(); eHandlerIter++)
					{
						(*eHandlerIter)(source, response);
					}
				}
			}
		}
	}
}
