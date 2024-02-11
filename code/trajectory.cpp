#include <fstream>
#include <vector>
#include "test.h"
#include "settings.h"
#include "nlohmann/json.hpp"
#include "walker.h"
#include "statics.h"

#define WALKER_FILE "/team17/trajectory.json"

using json = nlohmann::json;

class WalkerTrajectory : public Test
{
public:
	Walker* walky;
	json dump;
	int state_i;

	WalkerTrajectory()
	{
		// re-create the initial Walker based on the set walker dump file
		std::ifstream inf(WALKER_FILE);
		dump = json::parse(inf);
		std::vector<WalkerState> dump0 = { dump.at(0) };
		walky = new Walker(dump0, m_world);

		state_i = 0;
	}

	void Step(Settings& settings) override
	{
		settings.m_hertz = SIM_HERTZ;
		settings.m_positionIterations = SIM_POS_ITER;
		settings.m_velocityIterations = SIM_VEL_ITER;

		// clock
		float time_estimate = m_stepCount * (1.0f / SIM_HERTZ);
		g_debugDraw.DrawString(5, m_textLine, "Simulation time = %f", 
								time_estimate);
		m_textLine += m_textIncrement;

		// x-position of head
		g_debugDraw.DrawString(5, m_textLine, "Position.X = %f", 
								walky->GetPositionX());
		m_textLine += m_textIncrement;

		// state index
		g_debugDraw.DrawString(5, m_textLine, "State # = %d", state_i);
		m_textLine += m_textIncrement;

		// update Walker motors as often as Walker->Simulate()
		if (m_stepCount % N_ITER_TIMESTEPS == 0)
		{
			if (state_i < dump.size())
			{
				// WalkerState dump_state = dump.at(state_i);
				// dump_state.Diff(WalkerState(walky)).Print();
				
				walky->SetMotorSpeeds(	(float) dump.at(state_i)["mspeeds"][0],
										(float) dump.at(state_i)["mspeeds"][1],
										(float) dump.at(state_i)["mspeeds"][2],
										(float) dump.at(state_i)["mspeeds"][3]);

				state_i++;
			}
			// pause after the last state
			else 
			{
				walky->SetMotorSpeeds(0.0f, 0.0f, 0.0f, 0.0f);

				settings.m_pause = true;
			}
		}

		// simulate an iteration each Testbed "step"
		// if (!settings.m_pause)
		// {
		// 	walky->Simulate();
		// }

		Test::Step(settings);
	}

	static Test* Create()
	{
		return new WalkerTrajectory;
	}
};

static int testIndex = RegisterTest("team17", "Walker", 
									WalkerTrajectory::Create);