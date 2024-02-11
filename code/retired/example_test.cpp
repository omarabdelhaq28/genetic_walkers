#include "test.h"

class ExampleTest : public Test
{
	public:
		ExampleTest() {}
	
		void Step(Settings& settings) override
		{
			// run physics simulation + testbed rendering
			Test::Step(settings);

			// draw text
			g_debugDraw.DrawString(5, m_textLine, "Example Test Text");
			m_textLine += 15;
		}

		static Test* Create()
		{
			return new ExampleTest;
		}
};

static int testIndex = RegisterTest("team17", "Example Test",
									ExampleTest::Create);