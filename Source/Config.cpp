#include <process.h>

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

				bool Container::_inited = false;

				Configs Container::settings;

				bool Container::Set(Config* config)
				{
					static bool firstInit = false;

					if (firstInit)
					{
						atexit(Release);
					}

					_inited = true;
					settings.push_back(config);

					return true;
				}

				void Container::Release()
				{
					Configs::iterator sIter;

					if (_inited) {
						for (sIter = settings.begin(); sIter != settings.end(); sIter++)
						{
							delete (*sIter);
						}
					}
				}
			}
		}
	}
}
