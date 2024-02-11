// based on the steps outlined here:
// https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html

#include <cstdio>
#include "box2d/box2d.h"
#include "box2d/b2_math.h"

// note that Box2D uses KMS units; Box2D is intended to model real-world orders 
// of magnitude, and using pixels for length, etc. produces incorrect results
#define SHAPE_HEIGHT 50.0f  // [m]
#define SHAPE_WIDTH 10.0f
#define MASS_DENSITY 1.0f   // [kg] / [m^2]

#define FRICTION_COEFF 0.3f

#define DT 1.0f / 60.0f	 	// simulation time step [s]
#define VEL_ITER 6			// velocity iterations per step
#define POS_ITER 2			// position iterations per step
#define SIM_ITER 100		// number of simulation steps

int main(void)
{
    // define the simulation's gravity vector
    b2Vec2 gravity(0.0f, -10.0f);

    // the 'b2World' object manages all simulation objects and uses the 
	// specified gravity
    b2World world(gravity);

    // a "body" (simulation object) is introduced via the following process:
    // 1. define the body's initial position, etc.
    // 2. add the body to the world object
    // 3. define "fixtures" (geometric shape, material properties)
    // 4. set the defined fixtures to the body

    b2BodyDef bodyDef;                              // (1)
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, -10.0f);

    b2Body* body = world.CreateBody(&bodyDef);      // (2)

    b2PolygonShape boxShape;                        // (3)
    boxShape.SetAsBox(SHAPE_HEIGHT, SHAPE_WIDTH);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = MASS_DENSITY;
    fixtureDef.friction = FRICTION_COEFF;

    body->CreateFixture(&fixtureDef);               // (4)

    // simulation requires a solver to compute velocities (impulses) and 
	// positions; however, since these computations are independent, multiple 
	// passes (iterations) are needed (PER time step) to converge on the 
	// correct solution
	printf("N\tX\tY\tTheta\n");
	for (int i = 0; i < SIM_ITER; i++) {
		world.Step(DT, VEL_ITER, POS_ITER);
		b2Vec2 position = body->GetPosition();
		float angle = body->GetAngle();

		// note that Box2D is a physis engine, and does not render anything on 
		// its own; the Box2D "testbed" (technically a separate application)
		// can visualize Box2D objects and trajectories
		printf("%3d\t%4.2f\t%4.2f\t%4.2f\n", i, position.x, position.y, angle);
	}

	return 0;
}