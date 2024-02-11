#ifndef WALKER_H
#define WALKER_H

#include <vector>
#include "box2d/box2d.h"
#include "nlohmann/json.hpp"
#include "statics.h"

// true if index is referring to the upper legs
#define is_upper_leg(i) i < 2

// get the index of the upper part of the same-sided leg
#define get_upper_leg(i) i < 2 ? 0 : (i-2)

#define DEFAULT_DUMP_FNAME "trajectory.json"

// this struct defines the physical parameters of a Walker; these parameters
// do *not* change between Walker::Simulate() calls
struct WalkerParameters
{
	b2Vec2 head_size;
	b2Vec2 upper_leg_size;
	b2Vec2 lower_leg_size;
	float mass_density;
	float max_torque;			// [ASSUME] all joints have the same maximum 
								// applicable torque

	nlohmann::json Serialize();
};

const WalkerParameters defaultParameters = {
	{ HEAD_SIZE_X, HEAD_SIZE_Y },
	{ LEG_SIZE_X, LEG_SIZE_Y },
	{ LEG_SIZE_X, LEG_SIZE_Y },
	MASS_DENSITY,
	MAX_MOTOR_TORQUE
};

class Walker;

// data needed to reconstruct simulations; Box2D is reportedly deterministic
// (https://box2d.org/documentation/md__d_1__git_hub_box2d_docs__f_a_q.html)
//
// each Walker should have one WalkerState per simulation run
class WalkerState
{
public:
	// note that world center (physics, CoM) != position (geometry)
	WalkerParameters wp;
	b2Vec2 headWorldCenter;
	float headAngle;
	b2Vec2 legsWorldCenter[N_LEG_PARAMS];
	float legsAngle[N_LEG_PARAMS];
	float mspeeds[N_LEG_PARAMS];			// set motor angular speed
	float jspeeds[N_LEG_PARAMS];			// joint angular speed
	float jangles[N_LEG_PARAMS];			// joint angles
	int state_index;
	
	WalkerState();
	WalkerState(Walker* base);
	WalkerState(nlohmann::json serial);
	nlohmann::json Serialize();
	WalkerState Diff(WalkerState cmp);
	void Print();
};

class Walker
{
private:
	// box2d world objects
	b2World *world;
	b2Body *groundBody;

	// box2d walker objects
	b2BodyDef headDef;
	b2PolygonShape headShape;
	b2FixtureDef headFixDef;
	b2BodyDef legsDef[N_LEG_PARAMS];
	b2PolygonShape legsShape[N_LEG_PARAMS];
	b2FixtureDef legsFixDef[N_LEG_PARAMS];
	b2RevoluteJointDef jointsDef[N_LEG_PARAMS];

	void Exist(b2World *w = nullptr);
	void Build_Head(WalkerParameters wp);
	void Build_Legs(WalkerParameters wp);
	void Build_Joints(WalkerParameters wp);
	void Build(WalkerParameters wp = defaultParameters);

public:
	WalkerParameters params;
	b2Body *head;
	b2Body *legs[N_LEG_PARAMS];
	b2RevoluteJoint *joints[N_LEG_PARAMS];
	std::vector<WalkerState> states;

	// set motor speeds per joint; note that the actual angular speed of a joint
	// is dependent on the motor's torque; 'mspeed' refers to the explicitly
	// SET motor speed, while a function like b2RevoluteJoint::GetJointSpeed()
	// gets the actual speed
	float mspeeds[N_LEG_PARAMS];

	Walker(WalkerParameters wp = defaultParameters, b2World* w0 = nullptr);
	Walker(std::vector<WalkerState> image, b2World* w0 = nullptr);
	~Walker();

	// functions needed for GA
	std::vector<float> GetMotorSpeeds();
	void SetMotorSpeeds(float mUpperLeft, float mUpperRight, 
						float mLowerLeft, float mLowerRight);
	void Simulate();
	float GetPositionX();
	float GetPositionY();
	float GetVelocityX();
	float GetVelocityY();

	void Dump(bool use_default_fname = true);
};

#endif