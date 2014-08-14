#include <cstdlib>
#include <ctime>

#include "Verify.h"

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Verify
			{
				void Simple::key(long newKey)
				{
					verifyKey = newKey;
				}

				int Simple::gen()
				{
					srand((int)time(0));

					return rand();
				}

				bool Simple::pair(int randomKey, int checkKey)
				{
					// If no key has set, that will be true;
					if (verifyKey == 0L)
					{
						return true;
					}

					if (verifyKey * randomKey == checkKey)
					{
						return true;
					}

					return false;
				}
			}
		}
	}
}
