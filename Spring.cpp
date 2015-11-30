#include "Spring.h"


Spring::Spring(Point *P_a, Point *P_b, double alength, cWorld *world)
{
	a = P_a;
	b = P_b;
	// initialize the spring
	spring = new cShapeLine(a->point->getPos(), b->point->getPos());

	// initialize the color of the spring
	spring->m_ColorPointA = cColorf(1.0f, 0.0f, 0.0f, 0.5f);

	// add the spring to the world
	world->addChild(spring);

	// initialize the length of the spring
	length = alength;
}

Spring::~Spring()
{
	// remove the spring
	delete spring;
}

void Spring::applyForce()
{
	double newDis = cSub(a->point->getPos(), b->point->getPos()).length();
	if (length < newDis)
	{
		// Apply a pull force to both point
		cVector3d force = b->point->getPos() - a->point->getPos();
		cNormalize(force);
		force.mul(k * (newDis - length));
		// acceleration
		a->applyAcceleration(force);
		b->applyAcceleration(cNegate(force));
	}
	else if (length > newDis)
	{
		// Apply a pull force to both point
		cVector3d force = b->point->getPos() - a->point->getPos();
		cNormalize(force);
		force.mul(k * (length - newDis));
		// acceleration
		b->applyAcceleration(force);
		a->applyAcceleration(cNegate(force));
	}
}

void Spring::updatePos()
{
	spring->m_pointA = a->point->getPos();
	spring->m_pointB = b->point->getPos();
}