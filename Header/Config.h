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
					static bool Debug;
					static long Threads;

					static Data::ParameterSet APIs;
				};
			}
		}
	}
}
