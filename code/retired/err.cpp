#include <iostream>
#include <vector>
#include "walker.h"
#include "statics.h"
#include "nlohmann/json.hpp"

#define MOTOR_SPEED 1.0f

// check the error involved in "copying" a Walker by copying values from their
// WalkerStates and simulating the motor speeds in a different simulation;
// Box2D is deterministic, so if the copying implementation is decent, the
// error should be close to 0

int main()
{
    Walker *walky1 = new Walker();

    // simulate one iteration for a Walker
    walky1->SetMotorSpeeds(MOTOR_SPEED, MOTOR_SPEED, MOTOR_SPEED, MOTOR_SPEED);
    walky1->Simulate();

    // create a second Walker from the resulting WalkerState of the first
    WalkerState ws1(walky1);
    std::vector<WalkerState> img = { ws1 };
    Walker *walky2 = new Walker(img);
    WalkerState ws2(walky2);

    // compare
    std::cout << "WalkerState 1:\n";
    ws1.Print();
    std::cout << "WalkerState 2:\n";
    ws2.Print();

    // WalkerState diff = ws1.Diff(ws2);
    // std::cout << "WalkerState 1.Diff(2):\n";
    // diff.Print();
    // diff = ws2.Diff(ws1);
    // std::cout << "WalkerState 2.Diff(1):\n";
    // diff.Print();
    
    delete walky1;
    delete walky2;
}