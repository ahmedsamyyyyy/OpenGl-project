#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;
class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};


Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_player;
Model_3DS model_obstacle;
Model_3DS model_bullet;

// Textures
GLTexture tex_ground;

//Global vector to track bullet positions
std::vector<Vector> bulletPositions; // Store multiple bullet positions

// Global variable to track bullet rotation angle
float bulletRotationAngle = 0.0f;

// Function to update bullet positions and rotation
void UpdateBullets()
{
	for (auto& bulletPosition : bulletPositions) {
		bulletPosition.z -= 0.1; // Move the bullet forward along the z-axis
	}
	bulletRotationAngle += 1.0f; // Increment rotation angle
	if (bulletRotationAngle >= 360.0f) {
		bulletRotationAngle -= 360.0f; // Keep the angle within 0-360 degrees
	}
}

// Timer function to update bullet positions
void Timer(int value)
{
	UpdateBullets(); // Update bullet positions
	glutPostRedisplay(); // Redraw the scene
	glutTimerFunc(16, Timer, 0); // Call Timer again after 16 ms (~60 FPS)
}





// Global variable to track player position
Vector playerPosition(0, 0, 0); // Initialize player position

bool isFirstPerson = false;  // First-person mode toggle
bool isThirdPerson = false;  // Third-person mode toggle

float firstPersonOffsetY = 2.0f;  // Height of the first-person camera
float firstPersonOffsetZ = 0.5f;  // Forward offset of the first-person camera

float thirdPersonDistance = 8.0f; // Distance behind the player
float thirdPersonHeight = 5.0f;   // Height of the third-person camera
float thirdPersonLookHeight = 1.0f; // Height of the look-at point
float playerY = 1.0f; // Default Y position of the player's base
// Camera view selection
enum View { TOP_VIEW, SIDE_VIEW, FRONT_VIEW };
View currentView = FRONT_VIEW;
// Camera position and orientation
bool mouseDown = false;          // Is the mouse button pressed?
float cameraYaw = 0.0f;          // Horizontal rotation of the camera
float cameraPitch = 0.0f;        // Vertical rotation of the camera
float mouseSensitivity = 0.01f; // Mouse sensitivity
int lastMouseX = -1, lastMouseY = -1; // Track last mouse position
float zoomSpeed = 1.0f;  // Speed of zooming
bool isZooming = false;  // Flag to indicate if zooming is active
int lastY = -1;          // Last mouse Y position
float zoomLevel = 45.0f;  // Default zoom level for the camera
float cameraX = 0.0f, cameraY = 10.0f, cameraZ = 50.0f;  // Default camera position
float lookX = 0.0f, lookY = 0.0f, lookZ = 0.0f;          // Look at point
float upX = 0.0f, upY = 1.0f, upZ = 0.0f;               // Up vector
bool arrowKeys[4] = { false, false, false, false };  // Tracks state of arrow keys: {UP, DOWN, LEFT, RIGHT}

// Player settings
float playerX = 1.0f, playerZ = 1.0f;
float playerAngle = 0.0f; // Initial angle rotated by 180 degrees

void setCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(zoomLevel, 1.0, 0.1, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (isFirstPerson) {
		// First-person camera fixed to player's position and orientation
		gluLookAt(
			playerX, playerY + firstPersonOffsetY, playerZ, // Position at player's height
			playerX - sin(playerAngle * M_PI / 180.0f), playerY + firstPersonOffsetY, playerZ - cos(playerAngle * M_PI / 180.0f), // Look backward (180 degrees)
			0.0f, 1.0f, 0.0f // Up vector
		);
	}
	else if (isThirdPerson) {
		// Third-person camera positioned behind the player
		gluLookAt(
			playerX, playerY + thirdPersonHeight, playerZ + thirdPersonDistance, // Fixed position behind
			playerX, playerY + thirdPersonLookHeight, playerZ, // Look at the player
			0.0f, 1.0f, 0.0f // Up vector
		);
	}
	else {
		gluLookAt(
			playerX, playerY + thirdPersonHeight, playerZ + thirdPersonDistance, // Fixed position behind
			playerX, playerY + thirdPersonLookHeight, playerZ, // Look at the player
			0.0f, 1.0f, 0.0f // Up vector
		);
	}
}
void updatePlayerMovement() {
	float deltaX = 0.0f;
	float deltaZ = 0.0f;

	float speed = 0.130f; // Speed changes if boost is active

	if (arrowKeys[0]) deltaZ += speed;   // Up arrow
	if (arrowKeys[1]) deltaZ -= speed;   // Down arrow
	if (arrowKeys[2]) deltaX -= speed;   // Left arrow
	if (arrowKeys[3]) deltaX += speed;   // Right arrow

	// Update the player position
	playerX += deltaX;
	playerZ += deltaZ;

	// Update the camera for the current view
	if (isFirstPerson) {
		float angleRad = playerAngle * M_PI / 180.0f;
		float offsetX = firstPersonOffsetZ * cos(angleRad);
		float offsetZ = firstPersonOffsetZ * sin(angleRad);

		cameraX = playerX + offsetX;
		cameraY = playerY + firstPersonOffsetY;
		cameraZ = playerZ + offsetZ;

		lookX = cameraX + cos(angleRad);
		lookY = cameraY;
		lookZ = cameraZ + sin(angleRad);
	}
	else if (isThirdPerson) {
		float angleRad = playerAngle * M_PI / 180.0f;
		float offsetX = thirdPersonDistance * cos(angleRad + M_PI); // Behind the player
		float offsetZ = thirdPersonDistance * sin(angleRad + M_PI);

		cameraX = playerX + offsetX;
		cameraY = playerY + thirdPersonHeight;
		cameraZ = playerZ + offsetZ;

		lookX = playerX;
		lookY = playerY + thirdPersonLookHeight;
		lookZ = playerZ;
	}

	//checkFenceCollision(); // Ensure the player stays within bounds
	glutPostRedisplay();   // Request a redraw
}
void specialKeyDown(int key, int x, int y) {
	if (key == GLUT_KEY_DOWN) arrowKeys[0] = true;
	if (key == GLUT_KEY_UP) arrowKeys[1] = true;
	if (key == GLUT_KEY_LEFT) arrowKeys[2] = true;
	if (key == GLUT_KEY_RIGHT) arrowKeys[3] = true;
	updatePlayerMovement();
}

void specialKeyUp(int key, int x, int y) {
	if (key == GLUT_KEY_DOWN) arrowKeys[0] = false;
	if (key == GLUT_KEY_UP) arrowKeys[1] = false;
	if (key == GLUT_KEY_LEFT) arrowKeys[2] = false;
	if (key == GLUT_KEY_RIGHT) arrowKeys[3] = false;
}
//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

struct Obstacle {
	float x, y, z; // Position of the obstacle
	float speed;   // Speed of movement along the z-axis
};

float respawnInterval = 3.0f;  // Interval in seconds between respawns
float timeSinceLastRespawn = 0.0f; // Tracks elapsed time since the last respawn


std::vector<Obstacle> obstacles; // List to hold all obstacles
int numObstacles = 5; // Number of obstacles
float zResetPosition = -50.0f; // Starting z-position
float zEndPosition = 50.0f;    // End z-position
float groundWidth = 20.0f;     // Width of the ground along x-axis

void updateObstacles(float deltaTime) {
	timeSinceLastRespawn += deltaTime;

	// Only respawn when the timer exceeds the interval
	if (timeSinceLastRespawn >= respawnInterval) {
		for (auto& obstacle : obstacles) {
			if (obstacle.z > zEndPosition) {
				obstacle.z = zResetPosition;
				obstacle.x = (rand() % int(groundWidth * 2)) - groundWidth; // New random x position
				obstacle.speed = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.5f; // New random speed
			}
		}
		timeSinceLastRespawn = 0.0f; // Reset the timer
	}

	// Update positions for all obstacles
	for (auto& obstacle : obstacles) {
		obstacle.z += obstacle.speed * deltaTime; // Move along z-axis
	}
}

void renderObstacles() {
	for (const auto& obstacle : obstacles) {
		glPushMatrix();
		glTranslatef(obstacle.x, obstacle.y, obstacle.z);
		glScalef(0.5, 0.5, 0.5);
		model_obstacle.Draw();
		glPopMatrix();
	}
}

// Add this function here
float getElapsedTime() {
	static int lastTime = glutGet(GLUT_ELAPSED_TIME);
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = currentTime - lastTime;
	lastTime = currentTime;
	return elapsed / 1000.0f; // Convert milliseconds to seconds
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	setCamera();
	// Draw Ground
	RenderGround();

	// Draw Tree Model
	glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();

	//// Draw house Model
	//glPushMatrix();
	//glRotatef(90.f, 1, 0, 0);
	//model_house.Draw();
	//glPopMatrix();

	// Update player position in the drawing function
	glPushMatrix();
	glTranslatef(playerX, playerY, playerZ); // Use playerX, playerY, playerZ instead of playerPosition
	glRotatef(playerAngle + 180.0f, 0, 1, 0);  // Rotate the player by 180 degrees
	model_player.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, -10);
	glScalef(0.5, 0.5, 0.5);
	model_obstacle.Draw();
	glPopMatrix();


	// Draw all bullets with rotation
	for (const auto& bulletPosition : bulletPositions) {
		glPushMatrix();
		glTranslatef(bulletPosition.x, bulletPosition.y, bulletPosition.z); // Use bulletPosition for translation
		glRotatef(-bulletRotationAngle, 1.0f, 0.0f, 0.0f); // Rotate around the y-axis
		glScalef(0.2, 0.2, 0.2);
		model_bullet.Draw(); // Draw the bullet model
		glPopMatrix();
	}


	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();

	// Calculate elapsed time
	float deltaTime = getElapsedTime();

	// Update and render obstacles
	updateObstacles(deltaTime);
	renderObstacles();;

	glutSwapBuffers();
	glutPostRedisplay(); // Trigger the next frame
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'F':
	case 'f': // Toggle first-person mode
		isFirstPerson = !isFirstPerson;
		break;

	case 't': // Toggle third-person view
	case 'T':
		isThirdPerson = !isThirdPerson;
		isFirstPerson = false; // Disable first-person mode
		break;
	case 's': // New case for throwing a new bullet
		bulletPositions.push_back(Vector(playerX, playerY, playerZ)); // Add new bullet at player's position
		break;
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets() {
	// Existing asset loading
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_player.Load("Models/player/Man N070315.3DS");
	model_obstacle.Load("Models/ball/Ball 3DS.3ds");
	model_bullet.Load("Models/bullet/Dagger.3ds");

	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);

	// Initialize obstacles
	srand(time(NULL)); // Seed for random number generation
	for (int i = 0; i < numObstacles; i++) {
		Obstacle obstacle;
		obstacle.x = (rand() % int(groundWidth * 2)) - groundWidth; // Random x between -groundWidth and groundWidth
		obstacle.y = 0.5f;  // Height above ground
		obstacle.z = zResetPosition + i * 10; // Spaced along the z-axis
		obstacle.speed = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.5f; // Random speed
		obstacles.push_back(obstacle);
	}
}


//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(specialKeyDown);
	glutSpecialUpFunc(specialKeyUp);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);


	glShadeModel(GL_SMOOTH);

	glutTimerFunc(0, Timer, 0); // Start the timer with a 60 FPS target

	glutMainLoop();
}
