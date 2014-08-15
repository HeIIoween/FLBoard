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

				Config* Container::settings;

				bool Container::Set(Config* config)
				{
					static bool firstInit = false;

					if (firstInit)
					{
						atexit(Release);
					}
					else
					{
						delete settings;
					}

					_inited = true;
					settings = config;

					return true;
				}

				void Container::Release()
				{
					if (_inited) {
						delete settings;
					}
				}

				Config* Container::Get()
				{
					if (!_inited) {
						settings = new Config();
						_inited = true;

						return settings;
					}

					return settings;
				}
			}
		}
	}
}
