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
					long verifyKey;
				};
			}
		}
	}
}
