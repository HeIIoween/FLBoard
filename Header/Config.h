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
					vector <Sync::Client*> Clients;

					Config() {};
					~Config() {};

					static bool Debug;
				};

				typedef vector <Config*> Configs;

				class Container
				{
				public:
					static bool Set(Config* config);
					static void Release();

				protected:
					static bool _inited;
					static Configs settings;
				};
			}
		}
	}
}
