#include <sstream>

#include "Misc.h"

using namespace std;
using namespace Json;

namespace Misc
{
	namespace Encode
	{
		wstring stringToWstring(string str)
		{
			wstringstream wss;

			wss << str.c_str();

			return wss.str();
		}

		string wstringToString(wstring wstr)
		{
			stringstream ss;

			ss << wstr.c_str();

			return ss.str();
		}

		string UTF8Encode(string str)
		{
			return UTF8Encode(stringToWstring(str));
		}

		// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte, Thanks
		string UTF8Encode(wstring str)
		{
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0, NULL, NULL);
			
			string strTo(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &str[0], (int)str.size(), &strTo[0], size_needed, NULL, NULL);

			return strTo;
		}

		wstring UTF8Decode(string &str)
		{
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
			wstring wstrTo(size_needed, 0);

			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
			return wstrTo;
		}
	}

	namespace JsonHelper
	{
		string GetValueStr(Json::Value &value, string key)
		{
			if (!value.isObject()) {
				return "";
			}

			if (value[key].isString())
			{
				return value[key].asString();
			}

			return "";
		}

		bool GetValueBool(Json::Value &value, string key)
		{
			if (!value.isObject()) {
				return false;
			}

			if (value[key].isBool())
			{
				return value[key].asBool();
			}

			return false;
		}

		uint GetValueUint(Json::Value &value, string key)
		{
			if (!value.isObject()) {
				return 0;
			}

			if (value[key].isUInt())
			{
				return value[key].asUInt();
			}

			return 0;
		}

		int GetValueInt(Json::Value &value, string key)
		{
			if (!value.isObject()) {
				return 0;
			}

			if (value[key].isInt())
			{
				return value[key].asInt();
			}

			if (value[key].isString())
			{
				return stoi(value[key].asString());
			}

			return 0;
		}
	}
}