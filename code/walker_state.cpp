#include <iostream>
#include "nlohmann/json.hpp"
#include "box2d/box2d.h"
#include "walker.h"

#define ABS(x) x < 0 ? (-1*x) : x
#define PCTDIFF(x, y) (100 * (ABS(x-y))) / ((x+y)/2)

using json = nlohmann::json;

WalkerState::WalkerState()
{
	wp = defaultParameters;
	headWorldCenter = b2Vec2(0.0f, 0.0f);
	headAngle = 0.0f;
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		legsWorldCenter[i] = b2Vec2(0.0f, 0.0f);
		legsAngle[i] = 0.0f;
		mspeeds[i] = 0.0f;
		jspeeds[i] = 0.0f;
		jangles[i] = 0.0f;
	}
	state_index = 0;
}

// WalkerState struct "snapshot" of an existing Walker
WalkerState::WalkerState(Walker *base)
{
	wp = base->params;
	headWorldCenter = base->head->GetWorldCenter();
	headAngle = base->head->GetAngle();

	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		legsWorldCenter[i] = base->legs[i]->GetWorldCenter();
		legsAngle[i] = base->legs[i]->GetAngle();
		mspeeds[i] = base->joints[i]->GetMotorSpeed();
		jspeeds[i] = base->joints[i]->GetJointSpeed();
		jangles[i] = base->joints[i]->GetJointAngle();
	}

	// [TODO]
	state_index = 0;
}

// JSON --> WalkerState struct
WalkerState::WalkerState(json serial)
{
	wp.head_size.x = serial["wp"]["head_size"]["x"];
	wp.head_size.y = serial["wp"]["head_size"]["y"];
	wp.upper_leg_size.x = serial["wp"]["upper_leg_size"]["x"];
	wp.upper_leg_size.y = serial["wp"]["upper_leg_size"]["y"];
	wp.lower_leg_size.x = serial["wp"]["lower_leg_size"]["x"];
	wp.lower_leg_size.y = serial["wp"]["lower_leg_size"]["y"];
	wp.mass_density = serial["wp"]["mass_density"];
	wp.max_torque = serial["wp"]["max_torque"];
	headWorldCenter.x = serial["headWorldCenter"]["x"];
	headWorldCenter.y = serial["headWorldCenter"]["y"];
	headAngle = serial["headAngle"];
	for (int i = 0; i < N_LEG_PARAMS; i++) 
	{
		legsWorldCenter[i].x = serial["legsWorldCenter"].at(i)["x"];
		legsWorldCenter[i].y = serial["legsWorldCenter"].at(i)["y"];
		legsAngle[i] = serial["legsAngle"][i];
		mspeeds[i] = serial["mspeeds"][i];
		jspeeds[i] = serial["jspeeds"][i];
		jangles[i] = serial["jangles"][i];
	}

	// [TODO]
	state_index = 0;
}

// WalkerState struct --> JSON
json WalkerState::Serialize()
{
	json ser = json({});
	ser["wp"] = wp.Serialize();
	ser["headWorldCenter"] = {
		{"x", headWorldCenter.x},
		{"y", headWorldCenter.y}
	};
	ser["headAngle"] = headAngle;
	ser["legsWorldCenter"] = json::array();
	ser["legsAngle"] = json::array();
	ser["mspeeds"] = json::array();
	ser["jspeeds"] = json::array();
	ser["jangles"] = json::array();
	for (int i = 0; i < N_LEG_PARAMS; i++) {
		ser["legsWorldCenter"].push_back({
			{ "x", legsWorldCenter[i].x },
			{ "y", legsWorldCenter[i].y }
		});
		ser["legsAngle"].push_back(legsAngle[i]);
		ser["mspeeds"].push_back(mspeeds[i]);
		ser["jspeeds"].push_back(jspeeds[i]);
		ser["jangles"].push_back(jangles[i]);
	}

	return ser;
}

// return a WalkerState containing the % differences between the two states
// [TODO] this isn't commutative??
WalkerState WalkerState::Diff(WalkerState cmp)
{
	// struct members wp and state_index are left at WalkerState
	// default values
	WalkerState diff;

	diff.headWorldCenter = b2Vec2(
							PCTDIFF(headWorldCenter.x, cmp.headWorldCenter.x),
							PCTDIFF(headWorldCenter.y, cmp.headWorldCenter.y));
	diff.headAngle = PCTDIFF(headAngle, cmp.headAngle);
	for (int i = 0; i < N_LEG_PARAMS; i++) 
	{
		diff.legsWorldCenter[i] = b2Vec2(
									PCTDIFF(legsWorldCenter[i].x,
											cmp.legsWorldCenter[i].x),
									PCTDIFF(legsWorldCenter[i].y,
											cmp.legsWorldCenter[i].y));
		diff.legsAngle[i] = PCTDIFF(legsAngle[i], cmp.legsAngle[i]);
		diff.mspeeds[i] = PCTDIFF(mspeeds[i], cmp.mspeeds[i]);
		diff.jspeeds[i] = PCTDIFF(jspeeds[i], cmp.jspeeds[i]);
		diff.jangles[i] = PCTDIFF(jangles[i], cmp.jangles[i]);
	}

	return diff;
}

// print WalkerState members in a human-readable form
void WalkerState::Print()
{
	// [TODO] WalkerParameters is just raw JSON
	// std::cout << "WalkerParameters:\n" << wp.Serialize() << std::endl;

	std::cout 	<< "state_index = " << state_index << std::endl;

	std::cout 	<< "headWorldCenter.x = " << headWorldCenter.x
				<< "\nheadWorldCenter.y = " << headWorldCenter.y << std::endl;

	std::cout	<< "headAngle = " << headAngle << std::endl;

	std::cout	<< "legsWorldCenter = \n[\n";
	for (int i = 0; i < N_LEG_PARAMS; i++) 
	{
		std::cout	<< "\t{ .x = " << legsWorldCenter[i].x
					<< ", \t.y = " << legsWorldCenter[i].y << " }\n";
	}
	std::cout	<< "]" << std::endl;

	std::cout	<< "legsAngle = [ ";
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		std::cout	<< legsAngle[i]	<< " ";
	}
	std::cout	<< "]" << std::endl;

	std::cout	<< "mspeeds = [ ";
	for (int i = 0; i < N_LEG_PARAMS; i++) 
	{
		std::cout	<< mspeeds[i] << " ";
	}
	std::cout	<< "]" << std::endl;

	std::cout	<< "jspeeds = [ ";
	for (int i = 0; i < N_LEG_PARAMS; i++) 
	{
		std::cout	<< jspeeds[i] << " ";
	}
	std::cout << "]" << std::endl;

	std::cout	<< "jangles = [ ";
	for (int i = 0; i < N_LEG_PARAMS; i++)
	{
		std::cout	<< jangles[i] << " ";
	}
	std::cout << "]" << std::endl;
}