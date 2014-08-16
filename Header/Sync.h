#include <queue>
#include <map>

#include "Http.h"
#include "Verify.h"
#include "Parameter.h"

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

#include "json\json.h"

#define SYNC_CLIENT_MAX_DELAY 600000
#define SYNC_CLIENT_MAX_QUEUE 5000
#define SYNC_CLIENT_DEFAULT_QUEUE 300

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Sync
			{
				typedef enum APIResponseStatus
				{
					FAILED,
					SUCCEED,
					ERROR_MESSAGE,
					NO_TASK,
					PERIOD_LIMIT,
					DISABLED,
					SKIPED,
					INVALID_TOKEN,
					INVALID_SECERT,
					INVALID_DATA,
					INVALID_RESPONSE,
					INVALID_ACK,
					FAILED_LOGIN,
					FAILED_LOGOUT,
					FAILED_STATUS,
				};

				typedef map <string, wstring> DataValue;
				typedef map <string, DataValue> DataItem;

				typedef void(*EventCallback)(wstring source, Data::Parameter parameter);

				typedef map <string, DataValue> Responses;
				typedef map <wstring, wstring> ResponseValue;

				typedef struct APIResponseTask
				{
					string Type;
					ResponseValue Data;
				} apiResponse;

				typedef vector <APIResponseTask> APIResponses;

				typedef struct APILogin
				{
					string URI;
					string Account;
					string Password;
					long Secret;
					uint Queue;
					vector <string> Operations;
					vector <string> Responses;
				} apiLogin;

				typedef struct APIServer
				{
					string Token;
					wstring Name;
					uint Delay;
					clock_t LastSent;
					uint QueueLimit;
				} apiToken;

				typedef struct APIResponsePackage
				{
					APIResponses Responses;
					wstring API;
				} apiResponsePackage;

				typedef struct vector <APIResponsePackage> APIResponsePackages;

				class Client
				{
				public:
					static APIServer Get(APILogin clientLogin);
					static void Release();
					static void Send(DataItem data);

					static bool Run(APIResponsePackages *packages);

				protected:
					typedef vector <Client*> Instances;
					typedef queue <DataItem> Queue;

					Client(APILogin apiLogin, APIServer &serverInfo);
					~Client();

					Verify::Simple verifyer;

					APILogin loginInfo;
					APIServer server;
					bool enabled;

					Queue sendingQueue;

					CRITICAL_SECTION queueSycLock;
					static CRITICAL_SECTION instanceStaticOptLock;

					uint skips;

					static bool inited;

					static Instances instances;

					APIResponseStatus login(APIServer &serverInfo, string &errorMessage);
					APIResponseStatus logoff();
					
					void setJsonCommonHeader(Http::HttpHandler& http);

					void addQueue(DataItem data);
					APIResponseStatus sendQueue(APIResponses* responses);

					APIResponseStatus sync(Json::Value root, APIResponses* responses, bool noRetry = false);

					bool isTaskAllowsResponse(string taskType);
					bool isAllowedSendOperate(string reqeustType);

					void Client::printError(wstring error);
				};

				class Listener
				{
				public:
					static void Listen(string eventName, EventCallback eventCallback);
					static void Run(APIResponsePackages &packages);

				protected:
					typedef vector <EventCallback> EventHandlers;

					typedef struct EventHandler 
					{
						EventHandlers Handlers;
					};

					typedef map <string, EventHandler> Events;

					static Events events;

					static void trigger(wstring source, string eventName, Data::Parameter response);

					static void Listener::printError(wstring error);
				};
			}
		}
	}
}
