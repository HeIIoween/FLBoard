#pragma once

#include <string>
#include <map>

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

using namespace std;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Data
			{
				typedef map <wstring, wstring> ParameterData;

				class EXPORT Parameter
				{
				public:
					Parameter() {};
					Parameter(ParameterData raw);
					~Parameter() {};

					void set(ParameterData raw) { param = raw; };

					bool exist(wstring key);

					wstring get(wstring key);
					int getInt(wstring key);
					bool getBool(wstring key);
					uint getUint(wstring key);

				protected:
					ParameterData param;
				};
			}
		}
	}
}