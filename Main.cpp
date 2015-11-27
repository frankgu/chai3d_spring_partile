//===========================================================================
/*
This file is part of the CHAI 3D visualization and haptics libraries.
Copyright (C) 2003-2009 by CHAI 3D. All rights reserved.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License("GPL") version 2
as published by the Free Software Foundation.

For using the CHAI 3D libraries with software that can not be combined
with the GNU GPL, and for taking advantage of the additional benefits
of our support services, please contact CHAI 3D about acquiring a
Professional Edition License.

\author    <http://www.chai3d.org>
\author    Francois Conti
\version   2.0.0 $Rev: 269 $
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
#include "chai3d.h"
#include "Point.h"
#include "Spring.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// DECLARED CONSTANTS
//---------------------------------------------------------------------------

// initial size (width/height) in pixels of the display window
const int WINDOW_SIZE_W = 512;
const int WINDOW_SIZE_H = 512;

// mouse menu options (right button)
const int OPTION_FULLSCREEN = 1;
const int OPTION_WINDOWDISPLAY = 2;


//---------------------------------------------------------------------------
// DECLARED VARIABLES
//---------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera that renders the world in a window display
cCamera* camera;

// a light source to illuminate the objects in the virtual scene
cLight *light;

// a little "chai3d" bitmap logo at the bottom of the screen
cBitmap* logo;

// width and height of the current window display
int displayW = 0;
int displayH = 0;

// a virtual point1 
Point* point1;
Point* point2;
Point* point3;

// the virtual spring
Spring* spring1;
Spring* spring2;
Spring* spring3;

// a virtual ground
cMesh* ground;

// four vertices for the ground
int vertices[4];

// status of the main simulation haptics loop
bool simulationRunning = false;

// simulation clock
cPrecisionClock simClock;

// root resource path
string resourceRoot;

// has exited haptics simulation thread
bool simulationFinished = false;

//---------------------------------------------------------------------------
// DECLARED MACROS
//---------------------------------------------------------------------------
// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//---------------------------------------------------------------------------
// DECLARED FUNCTIONS
//---------------------------------------------------------------------------

// callback when the window display is resized
void resizeWindow(int w, int h);

// callback when a keyboard key is pressed
void keySelect(unsigned char key, int x, int y);

// callback when the right mouse button is pressed to select a menu item
void menuSelect(int value);

// function called before exiting the application
void close(void);

// main graphics callback
void updateGraphics(void);

// main haptics loop
void updateHaptics(void);


//===========================================================================
/*
DEMO:    cubic.cpp

This example illustrates how to build a small cube by assembling a
cloud of triangles together. The applications also presents the
use of texture properties by defining a texture image and defining
texture coordinates at each of the vertices of the object.
The texture image is produced and updated by copying the image
buffer of the virtual camera at each graphical rendering cycle.
*/
//===========================================================================

int main(int argc, char* argv[])
{
	//-----------------------------------------------------------------------
	// INITIALIZATION
	//-----------------------------------------------------------------------

	printf("\n");
	printf("-----------------------------------\n");
	printf("CHAI 3D\n");
	printf("Demo: 25-cubic\n");
	printf("Copyright 2003-2009\n");
	printf("-----------------------------------\n");
	printf("\n\n");
	printf("Keyboard Options:\n\n");
	printf("[x] - Exit application\n");
	printf("\n\n");

	// parse first arg to try and locate resources
	resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);


	//-----------------------------------------------------------------------
	// 3D - SCENEGRAPH
	//-----------------------------------------------------------------------

	// create a new world.
	world = new cWorld();

	// set the background color of the environment
	// the color is defined by its (R,G,B) components.
	world->setBackgroundColor(0, 0, 0);

	// create a camera and insert it into the virtual world
	camera = new cCamera(world);
	world->addChild(camera);

	// position and oriente the camera
	camera->set(cVector3d(3.0, 0.0, 0.0),    // camera position (eye)
		cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
		cVector3d(0.0, 0.0, 1.0));   // direction of the "up" vector

	// set the near and far clipping planes of the camera
	// anything in front/behind these clipping planes will not be rendered
	camera->setClippingPlanes(0.01, 10.0);

	// create a light source and attach it to the camera
	light = new cLight(world);
	camera->addChild(light);                   // attach light to camera
	light->setEnabled(true);                   // enable light source
	light->setPos(cVector3d(2.0, 0.5, 1.0));  // position the light source
	light->setDir(cVector3d(-2.0, 0.5, 1.0));  // define the direction of the light beam


	//-----------------------------------------------------------------------
	// 2D - WIDGETS
	//-----------------------------------------------------------------------

	// create a 2D bitmap logo
	logo = new cBitmap();

	// add logo to the front plane
	camera->m_front_2Dscene.addChild(logo);

	// load a "chai3d" bitmap image file
	bool fileload;
	fileload = logo->m_image.loadFromFile(RESOURCE_PATH("resources/images/chai3d-w.bmp"));
	if (!fileload)
	{
#if defined(_MSVC)
		fileload = logo->m_image.loadFromFile("../../../bin/resources/images/chai3d-w.bmp");
#endif
	}

	// position the logo at the bottom left of the screen (pixel coordinates)
	logo->setPos(10, 10, 0);

	// scale the logo along its horizontal and vertical axis
	logo->setZoomHV(0.25, 0.25);

	// here we replace all wite pixels (1,1,1) of the logo bitmap
	// with transparent black pixels (1, 1, 1, 0). This allows us to make
	// the background of the logo look transparent.
	logo->m_image.replace(
		cColorb(0xff, 0xff, 0xff),         // original RGB color
		cColorb(0xff, 0xff, 0xff, 0x00)    // new RGBA color
		);

	// enable transparency
	logo->enableTransparency(true);


	//-----------------------------------------------------------------------
	// POINTS/ HAPTIC DEVICES / TOOLS
	//-----------------------------------------------------------------------

	// create a 3D points and add it to the world
	point1 = new Point(0.05, world);
	point2 = new Point(0.05, world);
	point3 = new Point(0.05, world);

	point1->point->setPos(cVector3d(0, 0, 0.4));
	point2->point->setPos(cVector3d(-0.3, 0.6, 0.4));
	point3->point->setPos(cVector3d(0.3, 0.6, 0.5));

	// create two line connect theses three points
	spring1 = new Spring(point1, point2, world);
	spring2 = new Spring(point2, point3, world);
	spring3 = new Spring(point1, point3, world);

	//-----------------------------------------------------------------------
	// COMPOSE THE VIRTUAL SCENE
	//-----------------------------------------------------------------------

	// create a virtual mesh
	ground = new cMesh(world);

	// add object to world
	world->addChild(ground);

	// set the position of the object at the center of the world
	ground->setPos(0.0, 0.0, 0.0);


	/////////////////////////////////////////////////////////////////////////
	// create the ground
	/////////////////////////////////////////////////////////////////////////
	const double HALFSIZE = 1;

	// face -x
	vertices[0] = ground->newVertex(-HALFSIZE, HALFSIZE, -0.5);
	vertices[1] = ground->newVertex(-HALFSIZE, -HALFSIZE, -0.5);
	vertices[2] = ground->newVertex(HALFSIZE, -HALFSIZE, -0.5);
	vertices[3] = ground->newVertex(HALFSIZE, HALFSIZE, -0.5);

	// create the angle 
	ground->newTriangle(vertices[0], vertices[1], vertices[2]);
	ground->newTriangle(vertices[0], vertices[2], vertices[3]);

	// set material properties to light gray
	ground->m_material.m_ambient.set(0.5f, 0.5f, 0.5f, 1.0f);
	ground->m_material.m_diffuse.set(0.7f, 0.7f, 0.7f, 1.0f);
	ground->m_material.m_specular.set(1.0f, 1.0f, 1.0f, 1.0f);
	ground->m_material.m_emission.set(0.0f, 0.0f, 0.0f, 1.0f);

	// compute normals
	ground->computeAllNormals();

	// display triangle normals
	ground->setShowNormals(true);

	// set length and color of normals
	ground->setNormalsProperties(0.1, cColorf(1.0, 0.0, 0.0), true);

	// compute a boundary box
	ground->computeBoundaryBox(true);

	// compute collision detection algorithm
	ground->createAABBCollisionDetector(1.01 * 0.05, true, false);



	//-----------------------------------------------------------------------
	// OPEN GL - WINDOW DISPLAY
	//-----------------------------------------------------------------------

	// initialize GLUT
	glutInit(&argc, argv);

	// retrieve the resolution of the computer display and estimate the position
	// of the GLUT window so that it is located at the center of the screen
	int screenW = glutGet(GLUT_SCREEN_WIDTH);
	int screenH = glutGet(GLUT_SCREEN_HEIGHT);
	int windowPosX = (screenW - WINDOW_SIZE_W) / 2;
	int windowPosY = (screenH - WINDOW_SIZE_H) / 2;

	// initialize the OpenGL GLUT window
	glutInitWindowPosition(windowPosX, windowPosY);
	glutInitWindowSize(WINDOW_SIZE_W, WINDOW_SIZE_H);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(updateGraphics);
	glutKeyboardFunc(keySelect);
	glutReshapeFunc(resizeWindow);
	glutSetWindowTitle("CHAI 3D");

	// create a mouse menu (right button)
	glutCreateMenu(menuSelect);
	glutAddMenuEntry("full screen", OPTION_FULLSCREEN);
	glutAddMenuEntry("window display", OPTION_WINDOWDISPLAY);
	glutAttachMenu(GLUT_RIGHT_BUTTON);


	//-----------------------------------------------------------------------
	// START SIMULATION
	//-----------------------------------------------------------------------

	// simulation in now running
	simulationRunning = true;

	// create a thread which starts the main haptics rendering loop
	cThread* hapticsThread = new cThread();
	hapticsThread->set(updateHaptics, CHAI_THREAD_PRIORITY_HAPTICS);

	// start the main graphics rendering loop
	glutMainLoop();

	// close everything
	close();

	// exit
	return (0);
}

//---------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
	// update the size of the viewport
	displayW = w;
	displayH = h;
	glViewport(0, 0, displayW, displayH);

	// update texture coordinates
	double txMin, txMax, tyMin, tyMax;
	if (displayW >= displayH)
	{
		double ratio = (double)displayW / (double)displayH;
		txMin = 0.5 * (ratio - 1.0) / ratio;
		txMax = 1.0 - txMin;
		tyMin = 0.0;
		tyMax = 1.0;
	}
	else
	{
		double ratio = (double)displayH / (double)displayW;
		txMin = 0.0;
		txMax = 1.0;
		tyMin = 0.5 * (ratio - 1.0) / ratio;
		tyMax = 1.0 - tyMin;
	}

	// update texture coordinates
	ground->getVertex(vertices[0])->setTexCoord(txMin, tyMin);
	ground->getVertex(vertices[1])->setTexCoord(txMax, tyMin);
	ground->getVertex(vertices[2])->setTexCoord(txMax, tyMax);
	ground->getVertex(vertices[3])->setTexCoord(txMin, tyMax);
	
}

//---------------------------------------------------------------------------

void keySelect(unsigned char key, int x, int y)
{
	// escape key
	if ((key == 27) || (key == 'x'))
	{
		// close everything
		close();

		// exit application
		exit(0);
	}
}

//---------------------------------------------------------------------------

void menuSelect(int value)
{
	switch (value)
	{
		// enable full screen display
	case OPTION_FULLSCREEN:
		glutFullScreen();
		break;

		// reshape window to original size
	case OPTION_WINDOWDISPLAY:
		glutReshapeWindow(WINDOW_SIZE_W, WINDOW_SIZE_H);
		break;
	}
}

//---------------------------------------------------------------------------

void close(void)
{
	// stop the simulation
	simulationRunning = false;

	// wait for graphics and haptics loops to terminate
	while (!simulationFinished) { cSleepMs(100); }
}

//---------------------------------------------------------------------------

void updateGraphics(void)
{
	// render world
	camera->renderView(displayW, displayH);

	// Swap buffers
	glutSwapBuffers();

	// check for any OpenGL errors
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

	// inform the GLUT window to call updateGraphics again (next frame)
	if (simulationRunning)
	{
		glutPostRedisplay();
	}
}

//---------------------------------------------------------------------------

void updateHaptics(void)
{
	// reset clock
	simClock.reset();

	// main haptic simulation loop
	while (simulationRunning)
	{
		// stop the simulation clock
		simClock.stop();

		// read the time increment in seconds
		double timeInterval = simClock.getCurrentTimeSeconds();

		// restart the simulation clock
		simClock.reset();
		simClock.start();

		point1->resetAcceleration();
		point2->resetAcceleration();
		point3->resetAcceleration();


		/*
		*	calculate the all different kinds of froce and apply them onto the points
		*/
		// apply the gravity for to the point
		point1->applyAcceleration(cVector3d(0, 0, -9.81));
		point2->applyAcceleration(cVector3d(0, 0, -9.81));
		point3->applyAcceleration(cVector3d(0, 0, -9.81));

		// calculate the spring force and apply them to the point accelaration
		spring1->applyForce();
		spring2->applyForce();
		spring3->applyForce();

		// update the velocity according to the acceleration 
		point1->updateVel(timeInterval);
		point2->updateVel(timeInterval);
		point3->updateVel(timeInterval);

		// update the veolocity of the point according to the collision
		point1->collisionUpdate(ground);
		point2->collisionUpdate(ground);
		point3->collisionUpdate(ground);


		// add some damping too
		point1->damping(timeInterval);
		point2->damping(timeInterval);
		point3->damping(timeInterval);

		// compute the next tranlation configuration of the point
		point1->updatePos();
		point2->updatePos(); 
		point3->updatePos();


		// update the position of the spring
		spring1->updatePos();
		spring2->updatePos();
		spring3->updatePos();

	}

	// exit haptics thread
	simulationFinished = true;
}

//---------------------------------------------------------------------------

