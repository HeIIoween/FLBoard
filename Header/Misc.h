#include <string>

#include "..\..\flhookplugin_sdk\headers\FLHook.h"

#include "json\json.h"

using namespace std;

namespace Misc
{
	namespace Encode
	{
		int stringToInt(string str);

		wstring stringToWstring(string str);
		string wstringToString(wstring wstr);

		string UTF8Encode(wstring str);
		string UTF8Encode(string str);

		wstring UTF8Decode(string &str);
	}

	namespace JsonHelper
	{
		string GetValueStr(Json::Value &value, string key);
		bool GetValueBool(Json::Value &value, string key);
		uint GetValueUint(Json::Value &value, string key);
		int GetValueInt(Json::Value &value, string key);
	}
}