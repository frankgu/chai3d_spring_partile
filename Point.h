#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
#include "chai3d.h"
//---------------------------------------------------------------------------

#pragma once
class Point
{
private:

	double radius;
	const double DAMPING_GAIN = 0.6;
	double m = 100000; // the weight of the point
	cVector3d velocity;	// the velocity of the point
	cVector3d acceleration; // the acceleration of the point

public:
	cShapeSphere *point;

	Point(double radius, cWorld *world);
	~Point();
	
	/*
		public function
	*/
	// update the acceleration according to the force value
	void applyAcceleration(cVector3d force);
	// reset the acceleration
	void resetAcceleration();
	// get the weight of th poing
	double getWeight() { return m; }
	// update the velocity according to the acceleration value
	void updateVel(double timeInterval);
	// collision detection of this point, check if the point touch the ground
	void collisionUpdate(cMesh * ground);
	// dampling
	void damping(double timeInterval);
	// update the position of the point accorrding to the velocity
	void updatePos();

};

