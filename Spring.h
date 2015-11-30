#include "Point.h"

#pragma once
class Spring
{
private:

	double length; // the original length of the spring
	double k = 100; // the k for this spring
	Point *a;
	Point *b;

public:

	cShapeLine *spring;

	Spring(Point *a, Point *b, double alength, cWorld *world);
	~Spring();

	// get the length of the spring
	double getLength() { return length; }
	// update the force and apply them to the point
	void applyForce();
	// update the position of the spring
	void updatePos();
};

