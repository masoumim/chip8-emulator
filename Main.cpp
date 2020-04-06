/*
Chip-8 Emulator
Author: Mark Masoumi
E-mail: masoumi.mark@gmail.com
Date: April 6 2020
*/

#include <iostream>
#include <chrono>
#include <thread>
#include <GL/freeglut.h>
#include "Chip8.h"

using namespace std;

chip8 mychip8;

int window;
int menuChoice = 0;
string romName;
string roms[] = {"","15PUZZLE","BLINKY","BRIX","CONNECT4","GUESS","HIDDEN","INVADERS","KALEID","MAZE","MERLIN","MISSILE","PONG","PONG2","PUZZLE","TANK","TETRIS","TICTAC","UFO","VERS","WIPEOFF"};

void renderPixels();
void runGame();
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void menu(int); //Switches rom based on menu choice
void createMenu(); //Creates a GLUT menu
std::chrono::time_point<std::chrono::system_clock> start, endtime; //Create two time points using system_clock as the time source

int main(int argc, char** argv) {
	
	//init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100); //Where the window appear on your screen
	glutInitWindowSize(640, 320); //The size of the window
	window = glutCreateWindow("CHIP-8");

	createMenu();

	//register callbacks
	glutIdleFunc(runGame);
	glutDisplayFunc(renderPixels); //glutDisplayFunc calls the render function
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);

	//enter GLUT event processing cycle
	glutMainLoop();

	return 0;
}

void runGame() {
	/*******************************************************************************************************************************
	Note: Chip8 clock speed is approx 540Hz has a display refresh rate of 60Hz.
	(540 cycles per second) / (60 frames per second) = 9 cycles per frame. This means the CPU does 9 cycles before drawing a frame. 
	I rounded 9 up to 10 and so I run the emulation cycle 10 times and then draw a frame. 
	540 cycles per second = 1.85185185185 milliseconds per cycle.
	1.85185185185 milliseconds x 10 = 18.5185185185 milliseconds
	********************************************************************************************************************************/

	start = std::chrono::system_clock::now();
	
	//Run a cycle of the emulation
	for (int i = 0; i < 10; i++) {
		mychip8.emulateCycle();
	}
	
	endtime = std::chrono::system_clock::now();
	std::chrono::duration<double,milli> elapsed = (endtime - start);
	std::chrono::duration<double, milli> cycle(18.5185185185);
	std::chrono::duration<double, milli> sleepTime = cycle - elapsed;

	std::this_thread::sleep_for(sleepTime);

	mychip8.decreaseTimers();
	
	glutPostRedisplay();
}


void renderPixels(void) {

	//Clears the color buffer (and depth buffer too?)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set up the "screen" size?
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 64.0, 32.0, 0.0);

	//Draw pixels
	mychip8.drawPixels();

	glutSwapBuffers();
}


void keyboardDown(unsigned char key, int x, int y)
{
	if (key == '1')		    mychip8.key[0x1] = 1;
	else if (key == '2')	mychip8.key[0x2] = 1;
	else if (key == '3')	mychip8.key[0x3] = 1;
	else if (key == '4')	mychip8.key[0xC] = 1;

	else if (key == 'q')	mychip8.key[0x4] = 1;
	else if (key == 'w')	mychip8.key[0x5] = 1;
	else if (key == 'e')	mychip8.key[0x6] = 1;
	else if (key == 'r')	mychip8.key[0xD] = 1;

	else if (key == 'a')	mychip8.key[0x7] = 1;
	else if (key == 's')	mychip8.key[0x8] = 1;
	else if (key == 'd')	mychip8.key[0x9] = 1;
	else if (key == 'f')	mychip8.key[0xE] = 1;

	else if (key == 'z')	mychip8.key[0xA] = 1;
	else if (key == 'x')	mychip8.key[0x0] = 1;
	else if (key == 'c')	mychip8.key[0xB] = 1;
	else if (key == 'v')	mychip8.key[0xF] = 1;
}

void keyboardUp(unsigned char key, int x, int y)
{
	if (key == '1')		    mychip8.key[0x1] = 0;
	else if (key == '2')	mychip8.key[0x2] = 0;
	else if (key == '3')	mychip8.key[0x3] = 0;
	else if (key == '4')	mychip8.key[0xC] = 0;

	else if (key == 'q')	mychip8.key[0x4] = 0;
	else if (key == 'w')	mychip8.key[0x5] = 0;
	else if (key == 'e')	mychip8.key[0x6] = 0;
	else if (key == 'r')	mychip8.key[0xD] = 0;

	else if (key == 'a')	mychip8.key[0x7] = 0;
	else if (key == 's')	mychip8.key[0x8] = 0;
	else if (key == 'd')	mychip8.key[0x9] = 0;
	else if (key == 'f')	mychip8.key[0xE] = 0;

	else if (key == 'z')	mychip8.key[0xA] = 0;
	else if (key == 'x')	mychip8.key[0x0] = 0;
	else if (key == 'c')	mychip8.key[0xB] = 0;
	else if (key == 'v')	mychip8.key[0xF] = 0;
}


void menu(int num) {
	if (num == 0) {
		glutDestroyWindow(window);
		exit(0);
	}
	else {
		romName = roms[num];
	}

	mychip8.initialize();
	mychip8.loadGame(romName);
}

void createMenu() {
	glutCreateMenu(menu);

	glutAddMenuEntry("15PUZZLE", 1);
	glutAddMenuEntry("BLINKY", 2);
	glutAddMenuEntry("BRIX", 3);
	glutAddMenuEntry("CONNECT4", 4);
	glutAddMenuEntry("GUESS", 5);
	glutAddMenuEntry("HIDDEN", 6);
	glutAddMenuEntry("INVADERS", 7);
	glutAddMenuEntry("KALEID", 8);
	glutAddMenuEntry("MAZE", 9);
	glutAddMenuEntry("MERLIN", 10);
	glutAddMenuEntry("MISSILE", 11);
	glutAddMenuEntry("PONG", 12);
	glutAddMenuEntry("PONG2", 13);
	glutAddMenuEntry("PUZZLE", 14);
	glutAddMenuEntry("TANK", 15);
	glutAddMenuEntry("TETRIS", 16);
	glutAddMenuEntry("TICTAC", 17);
	glutAddMenuEntry("UFO", 18);
	glutAddMenuEntry("VERS", 19);
	glutAddMenuEntry("WIPEOFF", 20);
	glutAddMenuEntry("Quit", 0);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}