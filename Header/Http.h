#include <map>
#include <utility>

#include "curl\curl.h"
#include "curl\easy.h"
#include "curl\curlbuild.h"

#define CLIENT_USER_AGENT "Board Plugin for FLHook"

using namespace std;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Http
			{
				class HttpException : public exception
				{
				public:
					HttpException()
					{
					}

					HttpException(const char *message) : eMessage(message)
					{
					}

					HttpException(const string &message) : eMessage(message)
					{
					}

					virtual ~HttpException() throw (){}

					const char* what() const throw (){
						return eMessage.c_str();
					}

				protected:
					string eMessage;
				};

				class HttpCURLException : public HttpException
				{
				public:
					HttpCURLException()
					{
						this->eMessage = "CURL got a unknown exception";
					}

					HttpCURLException(CURLcode code)
					{
						this->eMessage = "CURL got a exception. Code: " + code;
					}
				};

				class HttpCURLInitException : public HttpCURLException
				{
				public:
					HttpCURLInitException()
					{
						this->eMessage = "CURL encountered unknown error when init";
					}

					HttpCURLInitException(CURLcode code)
					{
						this->eMessage = "CURL encountered a problem when init, error:" + code;
					}
				};

				typedef enum Method {
					HTTP_POST_METHOD,
					HTTP_GET_METHOD
				} method;

				class Http
				{
				public:
					// Creater and Decreater
					static Http *Create(string target);
					static void Free(Http* http);

					void setHeader(const char* header);
					void setMethod(Method method);

					void setData(string name, string data);

					bool send();

					string getResult();
					long getCode();

				protected:
					// curl easy instance
					CURL *curl;

					method currentMethod;

					// curl
					struct curl_slist *headers;

					// Target url
					string serverAddress;

					typedef map<string, string> QueryData;

					QueryData addressQuerys;

					string lastResponse;
					long lastStatusCode;

					Http(string target);
					~Http();

					static void Http::CleanUp();

					string getQueryDatas();

					// Curl handlers
					string curlEscape(string str);
					static size_t Http::staticCurlCallback(void* buf, size_t size, size_t nmemb, void* self);
					size_t Http::curlWriteCallback(char* buf, size_t size, size_t nmemb);
				};

				class HttpHandler
				{
				public:
					void data(string name, string value);
					void header(char* value);
					string result();
					long status();

				protected:
					Http* http;
				};

				class Get : public HttpHandler
				{
				public:
					Get(string url);
					~Get();
				};

				class Post : public HttpHandler
				{
				public:
					Post(string url);
					~Post();
				};
			}
		}
	}
}