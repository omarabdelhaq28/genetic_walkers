#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include "box2d/box2d.h"
#include "nlohmann/json.hpp"
#include "walker.h"

using json = nlohmann::json;

// initialize/set the Box2D world and add the ground
void Walker::Exist(b2World *w)
{
	if (!w)
	{
		world = new b2World(b2Vec2(0.0f, GRAVITY_Y));
	}
	else
	{
		world = w;
		world->SetGravity(b2Vec2(0.0f, GRAVITY_Y));
	}

	// define the ground as a static body
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0.0f, GROUND_Y - GROUND_SIZE_Y / 2);
	groundBody = world->CreateBody(&groundBodyDef);
	b2PolygonShape groundShape;
	groundShape.SetAsBox(GROUND_SIZE_X / 2, GROUND_SIZE_Y / 2);
	groundBody->CreateFixture(&groundShape, 0.0f);
}

void Walker::Build_Head(WalkerParameters wp)
{
	// [ASSUME] both legs are the same lengths
	float height = 	wp.head_size.y / 2 
					+ wp.upper_leg_size.y 
					+ wp.lower_leg_size.y;

	headDef.type = b2_dynamicBody;
	headDef.position.Set(0.0f, GROUND_Y + height);
	head = world->CreateBody(&headDef);
	headShape.SetAsBox(wp.head_size.x / 2, wp.head_size.y / 2);
	headFixDef.shape = &headShape;
	headFixDef.density = wp.mass_density;
	headFixDef.friction = FRICTION_COEFF;
	head->CreateFixture(&headFixDef);

	// [TODO] redundant? if any of the Build() functions are called separately,
	// without this line there is no guarantee that Walker::params will be set
	params = wp;
}

void Walker::Build_Legs(WalkerParameters wp)
{
	// legs
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		legsDef[i].type = b2_dynamicBody;
		{
			b2Vec2 ref;
			float x, y;
			// upper legs are attached directly below the head, equally spaced
			if (is_upper_leg(i))
			{
				ref = head->GetWorldCenter();
				x = ref.x + ((i % 2) * wp.head_size.x / 2) - wp.head_size.x / 4;
				y = ref.y - wp.head_size.y;
			}
			else
			{
				// lower legs are directly below each of their upper halves
				ref = legs[get_upper_leg(i)]->GetWorldCenter();
				x = ref.x;
				y = ref.y - wp.head_size.y;
			}
			legsDef[i].position.Set(x, y);
		}
		legs[i] = world->CreateBody(&legsDef[i]);

		if (is_upper_leg(i)) 
		{
			legsShape[i].SetAsBox(wp.upper_leg_size.x / 2, wp.upper_leg_size.y / 2);
		}
		else
		{
			legsShape[i].SetAsBox(wp.lower_leg_size.x / 2, wp.lower_leg_size.y / 2);
		}

		legsFixDef[i].shape = &legsShape[i];
		legsFixDef[i].density = wp.mass_density;
		legsFixDef[i].friction = FRICTION_COEFF;
		legs[i]->CreateFixture(&legsFixDef[i]);
	}

	// [TODO] redundant?
	params = wp;
}

void Walker::Build_Joints(WalkerParameters wp)
{
	// note that the use of two separate loops for legs/joints is just for clarity
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		// upper joint connects head and upper leg
		if (is_upper_leg(i))
		{
			jointsDef[i].bodyA = head;
			jointsDef[i].bodyB = legs[i];
			jointsDef[i].collideConnected = false;
		}
		else
		{
			// lower joint connects upper leg and lower leg (i.e. the knee)
			jointsDef[i].bodyA = legs[get_upper_leg(i)];
			jointsDef[i].bodyB = legs[i];
			jointsDef[i].collideConnected = false;
		}

		{
			if (is_upper_leg(i))
			{
				float ax = ((i % 2) * wp.head_size.x / 2) - wp.head_size.x / 4;
				float ay = -wp.head_size.y / 2;
				jointsDef[i].localAnchorA.Set(ax, ay);
				float bx = 0.0f;
				float by = wp.upper_leg_size.y / 2;
				jointsDef[i].localAnchorB.Set(bx, by);
			}
			else
			{
				float ax = 0.0f;
				float ay = -wp.upper_leg_size.y / 2;
				jointsDef[i].localAnchorA.Set(ax, ay);
				float bx = 0.0f;
				float by = wp.lower_leg_size.y / 2;
				jointsDef[i].localAnchorB.Set(bx, by);
			}
		}
		// note that the starting position of all half-legs should be vertical
		jointsDef[i].referenceAngle = 0;
		jointsDef[i].lowerAngle = MIN_JOINT_ANGLE;
		jointsDef[i].upperAngle = MAX_JOINT_ANGLE;
		jointsDef[i].enableLimit = true;
		jointsDef[i].enableMotor = true;
		jointsDef[i].maxMotorTorque = wp.max_torque;
		joints[i] = (b2RevoluteJoint *)world->CreateJoint(&jointsDef[i]);
	}

	// initialize motor speeds
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		mspeeds[i] = 0.0f;
	}

	// [TODO] redundant?
	params = wp;
}

// insantiate and align Walker rigid body components
void Walker::Build(WalkerParameters wp)
{
	Build_Head(wp);
	Build_Legs(wp);
	Build_Joints(wp);

	// set the 0th WalkerState
	states.push_back(WalkerState(this));
}

Walker::Walker(WalkerParameters wp, b2World *w0)
{
	Exist(w0);
	Build(wp);
}

Walker::Walker(std::vector<WalkerState> image, b2World* w0)
{
	Exist(w0);

	if (image.size() > 0)
	{
		WalkerState current = image.back();

		// build the child in the parent's image
		// [ASSUME] WalkerParameters should not change between Walker::Simulate() calls
		params = current.wp;
		Build_Head(params);
		Build_Legs(params);

		// apply transforms to put this new Walker in the same orientation as the
		// most recent image state
		head->SetTransform(current.headWorldCenter, current.headAngle);

		// note that joint speeds are measured (bodyB ang. vel.) - (bodyA ang. vel.)
		// [ASSUME] the ang. vel. of the head is 0
		float angVelocities[N_LEG_PARAMS];
		angVelocities[0] = current.jspeeds[0];
		angVelocities[1] = current.jspeeds[1];
		angVelocities[2] = angVelocities[0] + current.jspeeds[2];
		angVelocities[3] = angVelocities[1] + current.jspeeds[3];

		// destroy the built legs and reconstruct them with the above angular 
		// velocities; the previous Walker::Build_Legs() call defines most of 
		// the legsDef member, so only angular velocities needs to be specified
		for (int i = 0; i < N_LEG_PARAMS; i++) 
		{
			world->DestroyBody(legs[i]);

			legsDef[i].angularVelocity = angVelocities[i];
			legs[i] = world->CreateBody(&legsDef[i]);
			legs[i]->CreateFixture(&legsFixDef[i]);

			// orient legs to match the most recent image state
			legs[i]->SetTransform(current.legsWorldCenter[i], current.legsAngle[i]);
		}

		// note that all the legs must be reconstructed in order to reconstruct
		// their joints, so this part is separate from the above
		Build_Joints(params);
		SetMotorSpeeds(	current.mspeeds[0], current.mspeeds[1], 
						current.mspeeds[2], current.mspeeds[3]);

		// [DEBUG]
		// current.Diff(WalkerState(this)).Print();

		// inherit the previous states of the parent but replace the most recent
		// with the new Walker's current state
		states = image;
		states.pop_back();
		states.push_back(WalkerState(this));
	}
	else
	{
		std::cout 	<< "[walker.cpp] image w/o any states was passed to Walker"
					<< " constructor" << std::endl;
	}
}

Walker::~Walker()
{
	if (world)
	{
		delete world;
	}
}

std::vector<float> Walker::GetMotorSpeeds()
{
	std::vector<float> t{mspeeds[0], mspeeds[1], mspeeds[2], mspeeds[3]};
	return t;
}

void Walker::SetMotorSpeeds(float mUpperLeft, float mUpperRight, float mLowerLeft, float mLowerRight)
{
	mspeeds[UPPER_LEFT] = mUpperLeft;
	mspeeds[UPPER_RIGHT] = mUpperRight;
	mspeeds[LOWER_LEFT] = mLowerLeft;
	mspeeds[LOWER_RIGHT] = mLowerRight;

	// apply motor speeds
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		joints[i]->SetMotorSpeed(mspeeds[i]);
	}
}

void Walker::Simulate()
{
	// run simulation
	for (int i = 0; i < N_ITER_TIMESTEPS; i++)
	{
		world->Step(SIM_TIMESTEP, SIM_VEL_ITER, SIM_POS_ITER);
	}

	// record state
	states.push_back(WalkerState(this));
}

float Walker::GetPositionX()
{
	b2Vec2 headWorldCenter = head->GetWorldCenter();
	return headWorldCenter.x;
}

float Walker::GetPositionY()
{
	b2Vec2 headWorldCenter = head->GetWorldCenter();
	return headWorldCenter.y;
}

float Walker::GetVelocityX()
{
	b2Vec2 headVelocity = head->GetLinearVelocity();
	return headVelocity.x;
}

float Walker::GetVelocityY()
{
	b2Vec2 headVelocity = head->GetLinearVelocity();
	return headVelocity.y;
}

void Walker::Dump(bool use_default_fname)
{
	std::string fname;

	if (use_default_fname)
	{
		fname.append(DEFAULT_DUMP_FNAME);
	}
	// randomize the dump filename with an ID
	else 
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		fname.append("trajectory-");
		fname.append(std::to_string(gen()));
		fname.append(".json");
	}

	std::ofstream outfile;
	outfile.open(fname);
	if (outfile)
	{
		json array = json::array();
		for (int i = 0; i < (int)states.size(); i++) 
		{
			array.push_back(states[i].Serialize());
		}

		outfile << array << std::endl;
	}

	outfile.close();
}