#include <string>

#include "Http.h"

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Http
			{
				Http *Http::Create(string target)
				{
					static bool inited = false;

					if (!inited)
					{
						CURLcode initResult = curl_global_init(CURL_GLOBAL_ALL);

						atexit(CleanUp);

						if (initResult != CURLE_OK)
						{
							throw HttpCURLInitException(initResult);
						}

						inited = true;
					}

					return new Http(target);
				}

				void Http::CleanUp()
				{
					curl_global_cleanup();
				}

				void Http::Free(Http *http)
				{
					delete http;
				}

				Http::Http(string target)
				{
					headers = NULL;
					serverAddress = target;

					lastResponse = "";
					lastStatusCode = 0;

					curl = curl_easy_init();

					curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
					curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
					curl_easy_setopt(curl, CURLOPT_VERBOSE, false);
					curl_easy_setopt(curl, CURLOPT_HEADER, false);
					curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
					curl_easy_setopt(curl, CURLOPT_USERAGENT, CLIENT_USER_AGENT);
				}

				Http::~Http()
				{
					curl_slist_free_all(headers);
					curl_easy_cleanup(curl);
				}

				string Http::getQueryDatas()
				{
					string queryString = "";

					for (QueryData::iterator iterator = addressQuerys.begin(); iterator != addressQuerys.end(); iterator++) 
					{
						if (iterator->first == "" && iterator->second == "")
						{
							continue;
						}
						// Send Raw
						else if (iterator->second == "")
						{
							queryString.append(iterator->first);
						}
						// Send pure data
						else if (iterator->first == "")
						{
							queryString.append(curlEscape(iterator->second));
						}
						// Send key => val pair data
						else
						{
							queryString.append(curlEscape(iterator->first));
							queryString.append("=");
							queryString.append(curlEscape(iterator->second));
						}

						queryString.append("&");
					}

					// Remove the last & when we can
					if (queryString != "")
					{
						queryString = queryString.substr(0, queryString.length() - 1);
					}

					return queryString;
				}

				string Http::curlEscape(string str)
				{
					char *escaped = curl_easy_escape(curl, str.c_str(), str.length());

					string result(escaped);

					curl_free(escaped);

					return result;
				}

				size_t Http::staticCurlCallback(void* buf, size_t size, size_t nmemb, void* self)
				{
					return static_cast<Http*>(self)->curlWriteCallback(static_cast<char*>(buf), size, nmemb);
				}

				size_t Http::curlWriteCallback(char* buf, size_t size, size_t nmemb)
				{
					lastResponse.append(buf, size * nmemb);

					return size * nmemb;
				}

				void Http::setMethod(Method method)
				{
					currentMethod = method;
				}

				void Http::setData(string name, string data)
				{
					addressQuerys[name] = data;
				}

				void Http::setHeader(const char* header)
				{
					headers = curl_slist_append(headers, header);
				}

				string Http::getResult()
				{
					return lastResponse;
				}

				long Http::getCode()
				{
					return lastStatusCode;
				}

				bool Http::send()
				{
					const char* serverAddr = serverAddress.c_str();
					string queryString = getQueryDatas();

					string queryedSubmit = "";

					if (lastStatusCode != 0)
					{
						return false;
					}

					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &staticCurlCallback);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

					switch (currentMethod)
					{
					case HTTP_GET_METHOD:
						queryedSubmit.append(serverAddr);

						if (queryedSubmit.find("/?") != string::npos)
						{
							queryedSubmit.append("&");
						}
						else
						{
							queryedSubmit.append("/?");
						}

						queryedSubmit.append(queryString);

						curl_easy_setopt(curl, CURLOPT_URL, queryedSubmit.c_str());
						curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
						break;

					case HTTP_POST_METHOD:
						curl_easy_setopt(curl, CURLOPT_URL, serverAddr);
						curl_easy_setopt(curl, CURLOPT_POST, true);
						curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryString.c_str());
						break;
					}

					CURLcode res = curl_easy_perform(curl);

					if (res != CURLE_OK) {
						return false;
					}

					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lastStatusCode);

					return true;
				}

				// Http handler
				void HttpHandler::data(string name, string value)
				{
					http->setData(name, value);
				}

				void HttpHandler::header(char* value)
				{
					http->setHeader(value);
				}

				long HttpHandler::status()
				{
					return http->getCode();
				}

				string HttpHandler::result()
				{
					if (http->send())
					{
						return http->getResult();
					}

					return "";
				}

				// Get
				Get::Get(string url)
				{
					http = Http::Create(url);

					http->setMethod(HTTP_GET_METHOD);
				}

				Get::~Get()
				{
					Http::Free(http);
				}

				// Post
				Post::Post(string url)
				{
					http = Http::Create(url);

					http->setMethod(HTTP_POST_METHOD);
				}

				Post::~Post()
				{
					Http::Free(http);
				}
			}
		}
	}
}