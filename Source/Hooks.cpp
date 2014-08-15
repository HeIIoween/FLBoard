#include "Plugin.h"
#include "Hooks.h"
#include "Thread.h"
#include "Config.h"
#include "Common.h"
#include "Misc.h"

using namespace std;
using namespace raincious::FLHookPlugin::Board;

bool bGlobalDebug;

EXPORT void LoadSettings()
{
	int boardIndex = 0;

	returncode = DEFAULT_RETURNCODE;

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\board.cfg";

	Config::Config* config = new Config::Config();

	INI_Reader ini;
	if (!ini.open(scPluginCfgFile.c_str(), false)) {
		Common::PrintConInfo(L"Can't read config file. Don't see any possibility to load. Aborted.");

		return;
	}

	while (ini.read_header())
	{
		if (ini.is_header("API"))
		{
			Sync::APILogin login;
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
				ConPrint(L"There is an error on configuration: At least one of the parameter (URI, Account, Password) is missing for [API] %d. Ignoring.\r\n", boardIndex);

				continue;
			}

			config->Clients.push_back(Sync::Client::Get(login));
		}
		else if (ini.is_header("General"))
		{
			while (ini.read_value())
			{
				if (ini.is_value("Debug"))
				{
					Config::Config::Debug = ini.get_value_bool(0);
				}
			}
		}
	}

	ini.close();

	if (boardIndex <= 0)
	{
		Common::PrintConInfo(L"No board information. Don't see why loading. Aborted.");

		return;
	}

	Config::Container::Set(config);

	Thread::Worker::Start();
}