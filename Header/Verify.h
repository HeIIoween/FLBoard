#define MAX_THREAD_WAITING_TIME 120

namespace raincious
{
	namespace FLHookPlugin
	{
		namespace Board
		{
			namespace Verify
			{
				class Simple
				{
				public:
					Simple() { verifyKey = 0; };
					~Simple() {};

					void key(long newKey);
					int gen();
					bool pair(int randomKey, int checkKey);

				protected:
					long verifyKey = 0;
				};
			}
		}
	}
}
