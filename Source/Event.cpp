#include "Event.h"
#include "Config.h"
#include "Common.h"
#include "Misc.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Event
			{
				EXPORT void Listen(string eventName, EventCallback callback)
				{
					Sync::Listener::Listen(eventName, callback);
				}

				EXPORT void Send(string type, EventData data)
				{
					EventData::iterator dIter;

					Sync::DataValue value;
					Sync::DataItem item;

					for (dIter = data.begin(); dIter != data.end(); dIter++)
					{
						value[dIter->first] = dIter->second;
					}

					item[type] = value;

					Sync::Client::Send(item);
				}

				EXPORT void Import(string scPluginCfgFile, bool emptyFail)
				{
					int boardIndex = 0;
					string boardIndexStr = "";
					string errorMessage = "";
					Data::ParameterSet parameters;

					INI_Reader ini;
					if (!ini.open(scPluginCfgFile.c_str(), false)) {
						Common::PrintConInfo(L"Can't read config file. Don't see any possibility to load. Aborted.", true);

						return;
					}

					while (ini.read_header())
					{
						if (ini.is_header("API"))
						{
							Sync::APILogin login;
							Sync::APIServer server;
							boardIndex++;

							while (ini.read_value())
							{
								if (ini.is_value("URI"))
								{
									login.URI = ini.get_value_string(0);
								}
								else if (ini.is_value("Account"))
								{
									login.Account = ini.get_value_string(0);
								}
								else if (ini.is_value("Password"))
								{
									login.Password = ini.get_value_string(0);
								}
								else if (ini.is_value("Secret"))
								{
									login.Secret = Misc::Encode::stringToInt(ini.get_value_string(0));
								}
								else if (ini.is_value("Operation"))
								{
									login.Operations.push_back(ini.get_value_string(0));
								}
								else if (ini.is_value("Response"))
								{
									login.Responses.push_back(ini.get_value_string(0));
								}
							}

							if (login.URI == "" || login.Account == "" || login.Password == "") {
								boardIndexStr = itos(boardIndex);

								Common::PrintConInfo(L"There is an error on configuration: At least one of the parameter (URI, Account, Password) is missing for [API] " + wstring(boardIndexStr.begin(), boardIndexStr.end()) + L". Ignoring.\r\n", true);

								continue;
							}

							try
							{
								server = Sync::Client::Get(login);

								Data::ParameterData data;

								data[L"Name"] = server.Name;
								data[L"LastSent"] = (uint)server.LastSent;
								data[L"Queue"] = server.QueueLimit;
								data[L"Delay"] = server.Delay;

								Data::Parameter parameter(data);

								parameters.push_back(parameter);
							}
							catch (exception &e)
							{
								errorMessage = e.what();

								Common::PrintConInfo(L"Got a failure when try to load setting for API " + wstring(login.URI.begin(), login.URI.end()) + L": " + wstring(errorMessage.begin(), errorMessage.end()));
							}
						}
					}

					Config::Config::APIs = parameters;

					ini.close();

					if (emptyFail && boardIndex <= 0)
					{
						Common::PrintConInfo(L"No board information. Don't see why loading. Aborted.", true);

						return;
					}
				}

				EXPORT Data::ParameterSet APIs()
				{
					return Config::Config::APIs;
				}
			}
		}
	}
}
