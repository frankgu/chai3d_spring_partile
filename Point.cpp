#include "Point.h"


Point::Point(double r, cWorld *world)
{
	// initialize the point
	point = new cShapeSphere(r);
	radius = r;
	// add the point to the world
	world->addChild(point);

	// initialize the poisition of the point 
	point->setPos(0, 0, 0);

	// initialize the velocity  and acceleration of the point
	velocity.zero();
	acceleration.zero();
}

Point::~Point()
{
	// remove the point
	delete point;
}

void Point::applyAcceleration(cVector3d force)
{
	acceleration.add(force / m);
}

void Point::resetAcceleration()
{
	acceleration.zero();
}

void Point::updateVel(double timeInterval)
{
	velocity.add(timeInterval * acceleration);
}

void Point::collisionUpdate(cMesh * ground)
{
	cCollisionRecorder *recorder = new cCollisionRecorder();
	cCollisionSettings *setting = new cCollisionSettings();

	bool collision = ground->getCollisionDetector()->
		computeCollision(cVector3d(0, 0, 0), cVector3d(point->getPos().x,
		point->getPos().y, point->getPos().z - radius), *recorder, *setting);
	if (collision)
	{
		// change the z axis velocity to positive 
		double zvel = velocity.z * (-0.9);
		velocity = cVector3d(velocity.x, velocity.y, zvel);
	}
}

void Point::damping(double timeInterval)
{
	velocity.mul(1.0 - DAMPING_GAIN * timeInterval);
}

void Point::updatePos()
{
	if (velocity.length() > CHAI_SMALL)
	{
		point->translate(velocity);
	}

}