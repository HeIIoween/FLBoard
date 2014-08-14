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
				bool Container::_inited = false;
				Config* Container::settings;

				bool Container::Set(Config* config)
				{
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
