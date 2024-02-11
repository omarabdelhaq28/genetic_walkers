#include "box2d/box2d.h"
#include "nlohmann/json.hpp"
#include "walker.h"

using json = nlohmann::json;

// WalkerParameters struct --> JSON
json WalkerParameters::Serialize()
{
	json ser = json({});
	ser["head_size"] = {
		{"x", head_size.x},
		{"y", head_size.y}
	};
	ser["upper_leg_size"] = {
		{"x", upper_leg_size.x},
		{"y", upper_leg_size.y}
	};
	ser["lower_leg_size"] = {
		{"x", lower_leg_size.x},
		{"y", lower_leg_size.y}
	};
	ser["mass_density"] = mass_density;
	ser["max_torque"] = max_torque;

	return ser;
}