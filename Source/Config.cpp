#include <process.h>

#include "Common.h"
#include "Misc.h"
#include "Config.h"

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Config
			{
				bool Config::Debug = false;
				long Config::Threads = 0;

				Data::ParameterSet Config::APIs;
			}
		}
	}
}
