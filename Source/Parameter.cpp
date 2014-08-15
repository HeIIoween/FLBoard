#include "Parameter.h"
#include "Misc.h"

using namespace raincious::FLHookPlugin::Board;

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Data
			{
				Parameter::Parameter(ParameterData raw)
				{
					param = raw;
				}

				bool Parameter::exist(wstring key)
				{
					ParameterData::iterator it = param.find(key);

					if (it == param.end())
					{
						return false;
					}

					return true;
				}

				wstring Parameter::get(wstring key)
				{
					if (!exist(key)) {
						return L"";
					}

					return param[key];
				}

				int Parameter::getInt(wstring key)
				{
					if (!exist(key)) {
						return 0;
					}

					return Misc::Encode::stringToInt(string(param[key].begin(), param[key].end()));
				}
				
				uint Parameter::getUint(wstring key)
				{
					if (!exist(key)) {
						return 0;
					}

					return (uint)Misc::Encode::stringToInt(string(param[key].begin(), param[key].end()));
				}

				bool Parameter::getBool(wstring key)
				{
					if (!exist(key)) {
						return false;
					}

					if (param[key] == L"")
					{
						return false;
					}

					if (param[key] == L"no")
					{
						return false;
					}

					return true;
				}
			}
		}
	}
}
