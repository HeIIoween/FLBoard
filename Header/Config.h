#include <string>
#include <vector>

#include "Sync.h"

using namespace std;
using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Config
			{
				class Config
				{
				public:
					static struct General {
						wstring URI = L"";
						wstring Account = L"";
						wstring Token = L"";
					} general;

					vector <Sync::Client*> Clients;

					Config() {};
					~Config() {};
				};

				class Container
				{
				public:
					static bool Set(Config* config);
					static Config* Get();
					static void Release();

				protected:
					static bool _inited;

					static Config* settings;
				};
			}
		}
	}
}
