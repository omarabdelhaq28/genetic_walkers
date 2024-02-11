#ifndef STATICS_H
#define STATICS_H

// a file full of constants

// to match its chromosome representation, walker leg parameters are given by
// arrays of size 4 with the below ordering
#define UPPER_LEFT 0
#define UPPER_RIGHT 1
#define LOWER_LEFT 2
#define LOWER_RIGHT 3
#define N_LEG_PARAMS 4

// course (level? world?) parameters
#define GRAVITY_Y -10.0f
#define GROUND_Y 0.0f
#define GROUND_SIZE_X 50.0f
#define GROUND_SIZE_Y 5.0f
#define FRICTION_COEFF 0.05f

// computation parameters
#define SIM_HERTZ 60.0f                                 // # simulation updates per second
#define SIM_TIMESTEP 1.0f / SIM_HERTZ                   // simulation time step [s]
#define SIM_VEL_ITER 10				                    // velocity iterations per step
#define SIM_POS_ITER 10				                    // position iterations per step
#define ITER_TIME 0.5f                                  // total time simulated each iteration [s]
#define N_ITER_TIMESTEPS (int) (ITER_TIME * SIM_HERTZ)  // number of time steps per iteration

#define HEAD_SIZE_X 4.0f
#define HEAD_SIZE_Y 2.0f
#define LEG_SIZE_X 0.5f
#define LEG_SIZE_Y 2.0f
#define MASS_DENSITY 1.0f

#define DEG2RAD(ang) ang * b2_pi / 180.0f

// radians
#define MIN_JOINT_ANGLE DEG2RAD(-360.0f)
#define MAX_JOINT_ANGLE DEG2RAD(360.0f)

#define MAX_MOTOR_TORQUE 1000.0f

// radians per second
#define MIN_MOTOR_SPEED -10.0f
#define MAX_MOTOR_SPEED 10.0f

// Define constants for the number of walkers and iterations.
#define NUM_WALKERS 1000
#define NUM_ITERATIONS 10

// Define constants for the mutation and crossover probabilities.
#define MUTATION_PROBABILITY 0.1f
#define MUTATE_SIZE MAX_MOTOR_SPEED / 20
#define CROSSOVER_PROBABILITY 0.8f

#define FITTEST_RATIO 0.3f

#endif