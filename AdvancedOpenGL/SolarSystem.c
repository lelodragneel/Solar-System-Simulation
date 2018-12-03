
/************************************************************************************

File: 			SolarSystem.c

Description:	Assignment 2 textured solar system simulation with IO control

Author:			Lawrence Ayoub

*************************************************************************************/


/* include the library header files */
#include <stdlib.h>
#include <stdio.h>
#include <freeglut.h>
#include <math.h>

#define textureWidth  256
#define textureHeight 256
#define MIN (GLfloat) -200.0 // minimum value (for stars random coords generation)
#define MAX (GLfloat) 200.0 // maximum value (for stars random coords generation)
#define RANDOM MIN+((GLfloat)rand()/(GLfloat)RAND_MAX)*(MAX-MIN) // generates number in range [MIN-MAX]
#define TOTAL_STARS 2000 // total number of stars to generate
#define SOLARSYSTEMZONE 80.0 // the radius from the origin (in 3 dimensions) free from stars
#define PI 3.141592653589793
#define ORBIT_ACCURACY 40 // number of lines for drawing the orbit (higher numbers = more circular)
#define TOTAL_CORONA 100 // the number of lines that represent the sun's corona
#define CORONA_INTENSITY 2.5 // how long the sun's corona lines are

GLfloat upVector[3] = { 0.0, 1.0, 0.0 }; // our up vector
GLfloat vec3LookAt[3] = { 0.0, 0.0, 0.0 }; // default camera looking vector
GLfloat vec3CameraPos[3] = { 0.0, 2.0, 20.0 }; // default camera position

typedef enum { false, true } bool; // a boolean in C
GLUquadric *quad;
GLfloat theta = 0.0; // angle of rotation
GLfloat stars[TOTAL_STARS][3]; // to store all star coordinates

GLfloat verticies[10000][3] = { { 0 } }; // enterprise verticies
GLint poly[10000][3] = { { 0 } }; // enterprise faces
GLint j = 0;
GLfloat shieldColor[3] = { 0.0, 0.7, 0.9 }; // array for shield color to change over time slightly

GLfloat cameraSpeedMul = 0.08; // multiplier for camera movements in all 3 degrees of freedom
GLfloat xOffset; // for camera movements on x-axis
GLfloat yOffset; // for camera movements on y-axis
GLfloat zOffset; // for camera movements on z-axis
bool translateLeft = false;
bool translateRight = false;
bool translateUp = false;
bool translateDown = false;
bool translateForward = false;
bool translateBackward = false;

// togglables with keyboard
bool isStarsActive = true;
bool isTrajectoriesActive = true;
bool isCoronaActive = true;
bool isShieldActive = false;
bool isPlanetNamesActive = true;

// SUN VARIABLES
GLfloat sunRadius = 1.5; // radius of the planet/star
GLuint sunTexture;
// EARTH VARIABLES
GLfloat earthDistance = 6.0; // distance from origin
GLfloat earthRadius = 0.5; // radius of the planet/star
GLfloat earthOrbitalSpeed = 3.6; // how fast it orbits along the trajectory
GLuint earthTexture;
// MOON VARIABLES
GLfloat moonDistance = 1.0; // distance from earth
GLfloat moonRadius = 0.15; // radius of the planet/star
GLfloat moonOrbitalSpeed = 1.0; // how fast it orbits along the trajectory
GLuint moonTexture;
// MERCURY VARIABLES
GLfloat mercuryDistance = 2.0; // distance from origin
GLfloat mercuryRadius = 0.2; // radius of the planet/star
GLfloat mercuryOrbitalSpeed = 4.0; // how fast it orbits along the trajectory
GLuint mercuryTexture;
// VENUS VARIABLES
GLfloat venusDistance = 4.0; // distance from origin
GLfloat venusRadius = 0.4; // radius of the planet/star
GLfloat venusOrbitalSpeed = 3.8; // how fast it orbits along the trajectory
GLuint venusTexture;
// MARS VARIABLES
GLfloat marsDistance = 8.0; // distance from origin
GLfloat marsRadius = 0.3; // radius of the planet/star
GLfloat marsOrbitalSpeed = 3.2; // how fast it orbits along the trajectory
GLuint marsTexture;
// JUPITER VARIABLES
GLfloat jupiterDistance = 10.0; // distance from origin
GLfloat jupiterRadius = 0.9; // radius of the planet/star
GLfloat jupiterOrbitalSpeed = 2.5; // how fast it orbits along the trajectory
GLuint jupiterTexture;
// SATURN VARIABLES
GLfloat saturnDistance = 12.0; // distance from origin
GLfloat saturnRadius = 0.8; // radius of the planet/star
GLfloat saturnOrbitalSpeed = 1.6; // how fast it orbits along the trajectory
GLuint saturnTexture;
// URANUS VARIABLES
GLfloat uranusDistance = 14.0; // distance from origin
GLfloat uranusRadius = 0.7; // radius of the planet/star
GLfloat uranusOrbitalSpeed = 0.9; // how fast it orbits along the trajectory
GLuint uranusTexture;
// NEPTUNE VARIABLES
GLfloat neptuneDistance = 16.0; // distance from origin
GLfloat neptuneRadius = 0.6; // radius of the planet/star
GLfloat neptuneOrbitalSpeed = 0.3; // how fast it orbits along the trajectory
GLuint neptuneTexture;
// PLUTO VARIABLES
GLfloat plutoDistance = 18.0; // distance from origin
GLfloat plutoRadius = 0.1; // radius of the planet/star
GLfloat plutoOrbitalSpeed = 0.2; // how fast it orbits along the trajectory
GLuint plutoTexture;


/************************************************************************

Function:		createTexture

Description:	Reads a bitmap file.

*************************************************************************/
GLuint createTexture(const char *imgBmp)
{
	GLuint texture = 0;
	int width, height;
	BYTE * data = NULL;
	FILE * file;
	width = 128;
	height = 128;

	fopen_s(&file, imgBmp, "rb");

	if (&file == NULL) return 0;
	data = (BYTE*)malloc(width * height * 3);

	fread(data, width * height * 3, 1, file);
	fclose(file);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
 
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_BGRA_EXT, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_TEXTURE_ENV_COLOR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	free(data);

	return texture;

}


/************************************************************************

Function:		drawTrajectories

Description:	Draw an orbital trajectory outline of every planet.

*************************************************************************/
void drawTrajectories() 
{

	// set color of orbit trajectories
	glColor3f(1.0, 1.0, 1.0);

	// orbit trajectory line width
	glLineWidth(1.0);

	// draw orbital trajectory of earth
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (earthDistance + sunRadius), 0.0 , sin(angle) * (earthDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of mercury
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (mercuryDistance + sunRadius), 0.0, sin(angle) * (mercuryDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of venus
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (venusDistance + sunRadius), 0.0, sin(angle) * (venusDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of mars
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (marsDistance + sunRadius), 0.0, sin(angle) * (marsDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of jupiter
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (jupiterDistance + sunRadius), 0.0, sin(angle) * (jupiterDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of saturn
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (saturnDistance + sunRadius), 0.0, sin(angle) * (saturnDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of uranus
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (uranusDistance + sunRadius), 0.0, sin(angle) * (uranusDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of neptune
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (neptuneDistance + sunRadius), 0.0, sin(angle) * (neptuneDistance + sunRadius));
	}
	glEnd();

	// draw orbital trajectory of pluto
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
		glVertex3f(cos(angle) * (plutoDistance + sunRadius), 0.0, sin(angle) * (plutoDistance + sunRadius));
	}
	glEnd();

}

void drawSun() {
	// push the current transformation state on the stack
	glPushMatrix();

	// draw the sun
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, sunTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, sunRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 1.6, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Sun");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawEarthAndMoon() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate it, always around Y-axis
	glRotatef(theta * earthOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + earthDistance, 0.0, 0);

	// draw the earth
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, earthTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, earthRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.6, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Earth");
	}

	//---------------------
	// Now we draw the moon that orbits around earth. So we don't reset the matrix.
	//---------------------

	// set color of orbit trajectories
	glColor3f(1.0, 1.0, 1.0);

	// orbit trajectory line width
	glLineWidth(1.0);

	// draw orbital trajectory of moon
	if (isTrajectoriesActive) {
		glBegin(GL_LINE_LOOP);
		for (GLfloat angle = 0.0; angle <= 2 * PI; angle += (2 * PI) / ORBIT_ACCURACY) {
			glVertex3f(cos(angle) * moonDistance, 0.0, sin(angle) * moonDistance);
		}
		glEnd();
	}

	// rotate it, always around Y-axis
	glRotatef(theta, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(moonDistance, 0, 0);

	// draw the moon
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, moonTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, moonRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.3, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Moon");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawMercury() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * mercuryOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + mercuryDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mercuryTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, mercuryRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.4, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Mercury");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawVenus() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * venusOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + venusDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, venusTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, venusRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.5, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Venus");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawMars() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * marsOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + marsDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, marsTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, marsRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.4, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Mars");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawJupiter() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * jupiterOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + jupiterDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, jupiterTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, jupiterRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 1.0, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Jupiter");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawSaturn() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * saturnOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + saturnDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, saturnTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, saturnRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 1.0, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Saturn");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawUranus() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * uranusOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + uranusDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, uranusTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, uranusRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 1.0, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Uranus");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawNeptune() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * neptuneOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + neptuneDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, neptuneTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, neptuneRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.8, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Neptune");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}

void drawPluto() {
	// push the current transformation state on the stack
	glPushMatrix();

	// rotate around the y axis
	glRotatef(theta * plutoOrbitalSpeed, 0.0, 1.0, 0.0);

	// move it
	glTranslatef(sunRadius + plutoDistance, 0.0, 0);

	// draw the planet
	quad = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, plutoTexture);
	gluQuadricTexture(quad, GL_TRUE);
	gluSphere(quad, plutoRadius, 20, 20);
	glDisable(GL_TEXTURE_2D);

	// draw planet's title name
	if (isPlanetNamesActive) {
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(0.0, 0.2, 0.0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Pluto");
	}

	// pop the previous transformation state from the stack
	glPopMatrix();
}


/************************************************************************

Function:		drawSunCorona

Description:	Draw random transparent lines around the sun on XY plane.

*************************************************************************/
void drawSunCorona()
{

	// corona line width
	glLineWidth(5.0);

	// loop while creating random lines around the sun
	for (int i = 0; i < TOTAL_CORONA; i++) {
		GLfloat t = 2 * PI * rand() / (RAND_MAX + 1);
		glBegin(GL_LINES);
			glColor4f(0.8, 0.8, 0.0, 1.0);
			glVertex3f(cos(t) * 1.4, sin(t) * 1.4, 0.0);
			glColor4f(0.8, 0.8, 0.0, 0.0);
			glVertex3f(cos(t) * CORONA_INTENSITY, sin(t) * CORONA_INTENSITY, 0.0);
		glEnd();
	}

}


/************************************************************************

Function:		cross

Description:	Cross product two given matricies

*************************************************************************/
GLfloat *cross(GLfloat a[3], GLfloat b[3])
{

	// cross product a X b
	static GLfloat c[3];
	(GLfloat) c[0] = (a[1] * b[2]) - (a[2] * b[1]);
	(GLfloat) c[1] = (a[2] * b[0]) - (a[0] * b[2]);
	(GLfloat) c[2] = (a[0] * b[1]) - (a[1] * b[0]);

	return c;

}


/************************************************************************

Function:		myDisplay

Description:	Display callback, clears frame buffer and depth buffer,
				then emulate the solar system among everything else.

*************************************************************************/
void myDisplay(void)
{

	// clear the screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the identity matrix into the model view matrix
	glLoadIdentity();

	// subtract both vectors to get the forward vector (the vector that connects to both vectors from head-to-tail)
	GLfloat forwardVector[3] = { vec3LookAt[0] - vec3CameraPos[0], vec3LookAt[1] - vec3CameraPos[1] , vec3LookAt[2] - vec3CameraPos[2] };

	if (translateForward) { 
		// move forward
		vec3CameraPos[0] += forwardVector[0] * cameraSpeedMul;
		vec3CameraPos[1] += forwardVector[1] * cameraSpeedMul;
		vec3CameraPos[2] += forwardVector[2] * cameraSpeedMul;
		vec3LookAt[0] += forwardVector[0] * cameraSpeedMul;
		vec3LookAt[1] += forwardVector[1] * cameraSpeedMul;
		vec3LookAt[2] += forwardVector[2] * cameraSpeedMul;
	}
	else if (translateBackward) { 
		// move backward
		vec3CameraPos[0] -= forwardVector[0] * cameraSpeedMul;
		vec3CameraPos[1] -= forwardVector[1] * cameraSpeedMul;
		vec3CameraPos[2] -= forwardVector[2] * cameraSpeedMul;
		vec3LookAt[0] -= forwardVector[0] * cameraSpeedMul;
		vec3LookAt[1] -= forwardVector[1] * cameraSpeedMul;
		vec3LookAt[2] -= forwardVector[2] * cameraSpeedMul;
	}

	if (translateLeft) {
		// move left
		GLfloat *leftVector = cross(upVector, forwardVector);
		vec3CameraPos[0] += leftVector[0] * cameraSpeedMul;
		vec3CameraPos[1] += leftVector[1] * cameraSpeedMul;
		vec3CameraPos[2] += leftVector[2] * cameraSpeedMul;
		vec3LookAt[0] += leftVector[0] * cameraSpeedMul;
		vec3LookAt[1] += leftVector[1] * cameraSpeedMul;
		vec3LookAt[2] += leftVector[2] * cameraSpeedMul;
	}
	else if (translateRight) {
		// move right
		GLfloat *rightVector = cross(forwardVector, upVector);
		vec3CameraPos[0] += rightVector[0] * cameraSpeedMul;
		vec3CameraPos[1] += rightVector[1] * cameraSpeedMul;
		vec3CameraPos[2] += rightVector[2] * cameraSpeedMul;
		vec3LookAt[0] += rightVector[0] * cameraSpeedMul;
		vec3LookAt[1] += rightVector[1] * cameraSpeedMul;
		vec3LookAt[2] += rightVector[2] * cameraSpeedMul;
	}

	if (translateUp) {
		// move up
		vec3CameraPos[0] += upVector[0] * cameraSpeedMul * 2;
		vec3CameraPos[1] += upVector[1] * cameraSpeedMul * 2;
		vec3CameraPos[2] += upVector[2] * cameraSpeedMul * 2;
		vec3LookAt[0] += upVector[0] * cameraSpeedMul * 2;
		vec3LookAt[1] += upVector[1] * cameraSpeedMul * 2;
		vec3LookAt[2] += upVector[2] * cameraSpeedMul * 2;
	} 
	else if (translateDown) {
		// move down
		vec3CameraPos[0] -= upVector[0] * cameraSpeedMul * 2;
		vec3CameraPos[1] -= upVector[1] * cameraSpeedMul * 2;
		vec3CameraPos[2] -= upVector[2] * cameraSpeedMul * 2;
		vec3LookAt[0] -= upVector[0] * cameraSpeedMul * 2;
		vec3LookAt[1] -= upVector[1] * cameraSpeedMul * 2;
		vec3LookAt[2] -= upVector[2] * cameraSpeedMul * 2;
	}

	// adjust camera position and what we're looking at
	gluLookAt(vec3CameraPos[0], vec3CameraPos[1], vec3CameraPos[2], vec3LookAt[0], vec3LookAt[1], vec3LookAt[2], 0.0, 1.0, 0.0);

	// draw stars in background
	if (isStarsActive) {
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POINTS);
		for (int i = 0; i < TOTAL_STARS; i++) {
			glColor3f((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
			glVertex3f(stars[i][0], stars[i][1], stars[i][2]);
		}
		glEnd();
	}

	// draw planets
	drawSun();
	drawEarthAndMoon();
	drawMercury();
	drawVenus();
	drawMars();
	drawJupiter();
	drawSaturn();
	drawUranus();
	drawNeptune();
	drawPluto();

	// draw orbital trajectories
	if (isTrajectoriesActive)
		drawTrajectories();

	// draw sun's fiery corona
	if (isCoronaActive)
		drawSunCorona();

	// draw the enterprise
	glColor3f(1.0, 0.5, 0.5);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(forwardVector[0], forwardVector[1] + 1.3, forwardVector[2] + 17); // adjust enterprise position
	for (int a = 0; a < j; a++) {
		glBegin(GL_POLYGON);
			glVertex3f(verticies[poly[a][0] - 1][0], verticies[poly[a][0] - 1][1], verticies[poly[a][0] - 1][2]);
			glVertex3f(verticies[poly[a][1] - 1][0], verticies[poly[a][1] - 1][1], verticies[poly[a][1] - 1][2]);
			glVertex3f(verticies[poly[a][2] - 1][0], verticies[poly[a][2] - 1][1], verticies[poly[a][2] - 1][2]);
		glEnd();
	}

	// draw shield around enterprise
	if (isShieldActive) {
		quad = gluNewQuadric();
		glColor4f(shieldColor[0], shieldColor[1], shieldColor[2], 0.6);
		glScalef(1.0, 1.0, 2.3); // make sphere oval to encapsulate the enterprise
		gluSphere(quad, 0.5, 20, 20);
	}

	// pop the previous transformation state from the stack
	glPopMatrix();

	// swap the drawing buffers
	glutSwapBuffers();

}


/************************************************************************

Function:		printControls

Description:	Print camera controls and other key bindings to the debug console.

*************************************************************************/
void printControls()
{
	printf("------ Scene Controls ------\n");
	printf("r:	toggle trajectories\n");
	printf("s:	toggle stars\n");
	printf("c:	toggle sun corona\n");
	printf("k:	toggle enterprise shields\n");
	printf("n:	toggle planet names\n");
	printf("esc:	exit game\n\n");

	printf("------ Camera Controls ------\n");
	printf("Up Arrow:	move up\n");
	printf("Down Arrow:	move down\n");
	printf("Left Arrow:	move left\n");
	printf("Right Arrow:	move right\n");
	printf("Page Up:	forward\n");
	printf("Page Down:	backward\n");
}


/************************************************************************

Function:		myIdle

Description:	Updates the animation when idle.

*************************************************************************/
void myIdle()
{

	// update the rotation around the selected axis
	theta += 0.01f;

	if (translateLeft) {
		xOffset -= 0.1;
		translateLeft = false;
	}

	if (translateRight) {
		xOffset += 0.1;
		translateRight = false;
	}

	if (translateUp) {
		yOffset += 0.1;
		translateUp = false;
	}

	if (translateDown) {
		yOffset -= 0.1;
		translateDown = false;
	}

	if (translateForward) {
		zOffset -= 0.1;
		translateForward = false;
	}

	if (translateBackward) {
		zOffset += 0.1;
		translateBackward = false;
	}

	// change shield color over time
	shieldColor[1] += 0.001;
	shieldColor[2] += 0.001;
	if (shieldColor[1] > 0.8)
		shieldColor[1] = 0.7;
	if (shieldColor[2] > 1.0)
		shieldColor[2] = 0.9;

	// redraw the new state
	glutPostRedisplay();
}


/************************************************************************

Function:		mySpecialKeyboard

Description:	Handle camera movement.

*************************************************************************/
void mySpecialKeyboard(unsigned char key, int x, int y) 
{
	if (key == GLUT_KEY_LEFT)
		translateLeft = true;

	if (key == GLUT_KEY_RIGHT)
		translateRight = true;

	if (key == GLUT_KEY_UP)
		translateUp = true;

	if (key == GLUT_KEY_DOWN)
		translateDown = true;

	if (key == GLUT_KEY_PAGE_UP)
		translateForward = true;

	if (key == GLUT_KEY_PAGE_DOWN)
		translateBackward = true;

}


/************************************************************************

Function:		myKeyboard

Description:	Handle key clicks to toggle different things.

*************************************************************************/
void myKeyboard(unsigned char key, int x, int y) 
{
	
	if (key == 27) // 27 is code for the ESCAPE button
		exit(0);

	if (key == 'r')
		isTrajectoriesActive = !isTrajectoriesActive;

	if (key == 's')
		isStarsActive = !isStarsActive;

	if (key == 'c')
		isCoronaActive = !isCoronaActive;

	if (key == 'k')
		isShieldActive = !isShieldActive;

	if (key == 'n')
		isPlanetNamesActive = !isPlanetNamesActive;
}


/************************************************************************

Function:		myReshape

Description:	Just prevent user from reshaping the window.

*************************************************************************/
void myReshape(int width, int height) 
{
	glutReshapeWindow(500, 500);
}


/************************************************************************

Function:		buildEnterprise

Description:	Read verticies and polygons from file into arrays.

*************************************************************************/
void buildEnterprise()
{

	// IMPORTANT
	// "fopen" will make compiler complain about being depricated.
	// To fix this, I wen to project configuration -> C/C++ -> Preprocessor
	// And for the "ProcessorDefinitions" field, I added ";_CRT_SECURE_NO_WARNINGS"
	FILE *file;
	char buff[255];
	int i = 1;

	file = fopen("enterprise.txt", "r");

	fscanf(file, "%s", buff);
	fscanf(file, "%s", buff);
	sscanf(buff, "%f", &verticies[0][0]);
	fscanf(file, "%s", buff);
	sscanf(buff, "%f", &verticies[0][1]);
	fscanf(file, "%s", buff);
	sscanf(buff, "%f", &verticies[0][2]);

	while (fscanf(file, "%s", buff) != EOF) {
		if (strcmp(buff, "v") == 0) {
			fscanf(file, "%s", buff);
			sscanf(buff, "%f", &verticies[i][0]);
			fscanf(file, "%s", buff);
			sscanf(buff, "%f", &verticies[i][1]);
			fscanf(file, "%s", buff);
			sscanf(buff, "%f", &verticies[i][2]);
			i++;
		}
		else {
			fscanf(file, "%s", buff);
			sscanf(buff, "%d", &poly[j][0]);
			fscanf(file, "%s", buff);
			sscanf(buff, "%d", &poly[j][1]);
			fscanf(file, "%s", buff);
			sscanf(buff, "%d", &poly[j][2]);
			j++;
		}
	}

	fclose(file);
}


/************************************************************************

Function:		initializeGL

Description:	Initializes the OpenGL rendering context for display.

*************************************************************************/
void initializeGL(void)
{

	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// set background color to be black
	glClearColor(0, 0, 0, 1.0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// change into projection mode so that we can change the camera properties
	glMatrixMode(GL_PROJECTION);

	// load the identity matrix into the projection matrix
	glLoadIdentity();

	// set camera to perspective
	gluPerspective(45.0, 1.0, 0.01, 300);

	// change into model-view mode so that we can change the object positions
	glMatrixMode(GL_MODELVIEW);

	// star size
	glPointSize(2.0);

	// enable and setup lighting
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat globalAmbient[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat position[] = { 0.0, 80.0, 0.0, 1.0 };
	GLfloat direction[] = { 1.0, 0.0, 1.0 };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	// set global ambience
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
	// set position and direction vectory of spotlight
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, direction);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 40);
}


/************************************************************************

Function:		main

Description:	Sets up the openGL rendering context and the windowing
				system, then begins the display loop.

*************************************************************************/
void main(int argc, char** argv)
{
	// initialize the toolkit
	glutInit(&argc, argv);
	// set display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// set window size
	glutInitWindowSize(500, 500);
	// set window position on screen
	glutInitWindowPosition(100, 100);
	// open the screen window
	glutCreateWindow("Solar System");

	// register redraw function
	glutDisplayFunc(myDisplay);
	// register the idle function
	glutIdleFunc(myIdle);
	// register the keyboard function
	glutKeyboardFunc(myKeyboard);
	// register the keyboard function for special keys
	glutSpecialFunc(mySpecialKeyboard);
	// register the window resize function
	glutReshapeFunc(myReshape);

	//initialize the rendering context
	initializeGL();

	// generate random star coordinates
	for (int i = 0; i < TOTAL_STARS; i++) {
		// generate star coordinates and ensure the star coordinates are 
		// not within the sphere-of-influence (SOLARSYSTEMZONE is the 3d radius)
		// I used the distance formula below to measure the magnitude of the vector
		do {
		stars[i][0] = RANDOM;
		stars[i][1] = RANDOM;
		stars[i][2] = RANDOM;
		} while (sqrtf(pow(stars[i][0], 2)+pow(stars[i][1], 2)+pow(stars[i][2], 2)) <= SOLARSYSTEMZONE);
	}

	// read verticies and polygons from file into arrays
	buildEnterprise();

	// print key bindings to debug console
	printControls();

	// read and initialize planet textures
	sunTexture = createTexture("planet_bitmaps/sun.bmp");
	earthTexture = createTexture("planet_bitmaps/earth.bmp");
	moonTexture = createTexture("planet_bitmaps/moon.bmp");
	mercuryTexture = createTexture("planet_bitmaps/mercury.bmp");
	venusTexture = createTexture("planet_bitmaps/venus.bmp");
	marsTexture = createTexture("planet_bitmaps/mars.bmp");
	jupiterTexture = createTexture("planet_bitmaps/jupiter.bmp");
	saturnTexture = createTexture("planet_bitmaps/saturn.bmp");
	uranusTexture = createTexture("planet_bitmaps/uranus.bmp");
	neptuneTexture = createTexture("planet_bitmaps/neptune.bmp");
	plutoTexture = createTexture("planet_bitmaps/pluto.bmp");

	// go into a perpetual loop 
	glutMainLoop();
}