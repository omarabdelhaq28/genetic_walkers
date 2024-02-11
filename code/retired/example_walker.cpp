#include "test.h"

// freely inspired by:
// https://github.com/openai/gym/blob/master/gym/envs/box2d/bipedal_walker.py
// https://pylessons.com/BipedalWalker-v3-PPO

// note that "H" = "Half"; "DIM_HX" = half-width
#define HEAD_DIM_HX 2.0f			// [m]
#define HEAD_DIM_HY 1.0f
#define LEG_DIM_HX 0.5f
#define LEG_DIM_HY 1.0f				// height of each half-leg
#define MASS_DENSITY 1.0f			// [kg] / [m^2]

// derivative definitions to make things simpler
#define HEAD_DIM_X 2*HEAD_DIM_HX
#define HEAD_DIM_QX HEAD_DIM_HX/2	// "Q" = "Quarter"
#define HEAD_DIM_Y 2*HEAD_DIM_HY
#define HEAD_DIM_QY HEAD_DIM_HY/2
#define LEG_DIM_X 2*LEG_DIM_HX
#define LEG_DIM_QX LEG_DIM_HX/2
#define LEG_DIM_Y 2*LEG_DIM_HY
#define LEG_DIM_QY LEG_DIM_HY/2

#define POSITION_Y -1.0f + 2*LEG_DIM_Y + HEAD_DIM_Y

#define FRICTION_COEFF 0.3f
#define DT 1.0f / 60.0f				// simulation time step [s]
#define VEL_ITER 6
#define POS_ITER 2
#define SIM_ITER 1000

// the "walker" consists of 1 head, 2 legs (each split into an "upper" and 
// "lower" half) and 4 joints; the 2 "upper" joints fix the legs to the head, 
// and the 2 "lower" joints are basically knees (connects "upper" and "lower" 
// halves of each leg)

class ExampleWalker : public Test
{
public:

	b2BodyDef groundDef;
	b2Body *ground;

	b2BodyDef headDef;
	b2Body *head;
	b2FixtureDef headFixDef;
	b2BodyDef legsDef[2][2];				// 0: left, upper | 1: right, lower
	b2Body *legs[2][2];
	b2FixtureDef legsFixDef[2][2];
	b2RevoluteJointDef jointsDef[2][2];
	b2RevoluteJoint *joints[2][2];

	ExampleWalker()
	{
		// define the ground as a static body
		groundDef.position.Set(0.0f, -10.0f);
		ground = m_world->CreateBody(&groundDef);
		b2PolygonShape groundShape;
		groundShape.SetAsBox(50.0f, 10.0f);
		ground->CreateFixture(&groundShape, 0.0f);
		
		// head
		headDef.type = b2_dynamicBody;
		headDef.position.Set(0.0f, POSITION_Y);
		head = m_world->CreateBody(&headDef);
		b2PolygonShape headShape;
		headShape.SetAsBox(HEAD_DIM_HX, HEAD_DIM_HY);
		headFixDef.shape = &headShape;
		headFixDef.density = MASS_DENSITY;
		head->CreateFixture(&headFixDef);

		b2PolygonShape shape0[2], shape1[2];
		for (int i = 0; i <= 1; i++) 
		{
			// upper half-leg
			legsDef[i][0].type = b2_dynamicBody;
			{
				b2Vec2 headWorldCenter = head->GetWorldCenter();
				float x = headWorldCenter.x + (i*HEAD_DIM_HX) - HEAD_DIM_QX;
				float y = headWorldCenter.y - HEAD_DIM_Y;
				legsDef[i][0].position.Set(x, y);
			}
			legs[i][0] = m_world->CreateBody(&legsDef[i][0]);
			shape0[i].SetAsBox(LEG_DIM_HX, LEG_DIM_HY);
			legsFixDef[i][0].shape = &shape0[i];
			legsFixDef[i][0].density = MASS_DENSITY;
			legs[i][0]->CreateFixture(&legsFixDef[i][0]);

			// lower half-leg
			legsDef[i][1].type = b2_dynamicBody;
			{
				b2Vec2 upperLegWorldCenter = legs[i][0]->GetWorldCenter();
				float x = upperLegWorldCenter.x;
				float y = upperLegWorldCenter.y - LEG_DIM_Y;
				legsDef[i][1].position.Set(x, y);
			}
			legs[i][1] = m_world->CreateBody(&legsDef[i][1]);

			//shape1[i].SetAsBox(LEG_DIM_X, LEG_DIM_Y);			// a more fashionable option
			shape1[i].SetAsBox(LEG_DIM_HX, LEG_DIM_HY);

			legsFixDef[i][1].shape = &shape1[i];
			legsFixDef[i][1].density = MASS_DENSITY;
			legs[i][1]->CreateFixture(&legsFixDef[i][1]);
		}

		// the use of two separate loops for legs/joints is just for clarity
		// [TODO] unfinished; see https://www.iforce2d.net/b2dtut/joints-revolute
		for (int i = 0; i <= 1; i++)
		{
			// upper joint
			jointsDef[i][0].bodyA = head;
			jointsDef[i][0].bodyB = legs[i][0];
			jointsDef[i][0].collideConnected = false;
			{
				float ax = (i*HEAD_DIM_HX) - HEAD_DIM_QX;
				float ay = -HEAD_DIM_HY;
				jointsDef[i][0].localAnchorA.Set(ax, ay);
				float bx = 0.0f;
				float by = LEG_DIM_HY;
				jointsDef[i][0].localAnchorB.Set(bx, by);
			}
			jointsDef[i][0].referenceAngle = 0;						// note that the starting position of all half-legs should be vertical
			jointsDef[i][0].enableMotor = true;
			joints[i][0] = (b2RevoluteJoint*) m_world->CreateJoint(&jointsDef[i][0]);

		// 	// lower joint
		jointsDef[i][1].bodyA = legs[i][0];
		jointsDef[i][1].bodyB = legs[i][1];
		jointsDef[i][1].collideConnected = false;
		{
			float ax = 0.0f;
			float ay = -LEG_DIM_HY;
			jointsDef[i][1].localAnchorA.Set(ax, ay);
			float bx = 0.0f;
			float by = LEG_DIM_HY;
			jointsDef[i][1].localAnchorB.Set(bx, by);
		}
		jointsDef[i][1].referenceAngle = 0;
		jointsDef[i][1].enableMotor = true;
		joints[i][1] = (b2RevoluteJoint*) m_world->CreateJoint(&jointsDef[i][1]);

		// note that no limit on joint angle has been set (legs can currently rotate 360)
		}

		float torques[2][2] = { { 1.0f, 1.0f },
								{ 2.0f, 2.0f } };

		for (int i = 0; i <= 1; i++)
			for (int j = 0; j <= 1; j++)
				joints[i][j]->SetMotorSpeed(torques[i][j]);
	}

	static Test* Create()
	{
		return new ExampleWalker;
	}
};

static int testIndex = RegisterTest("team17", "Example Walker", ExampleWalker::Create);