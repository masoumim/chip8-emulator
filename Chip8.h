/*
Chip-8 Emulator
Author: Mark Masoumi
E-mail: masoumi.mark@gmail.com
Date: April 6 2020
*/

#include <random>

using namespace std;

class chip8 {
	//Member Variables:
	
	//Store C8 opcodes as Short (2 Bytes)
	unsigned short opcode;

	//C8 has 4K memory. 1K = 1024 Bytes. 4K = 4096 Bytes. Char = 1 Byte.
	unsigned char memory[4096] = { 0 };

	//C8 has 16 general-purpose, 8-bit registers referred to to as V0 to VF.
	unsigned char V[16] = { 0 };

	//Index register aka Memory Address Register
	unsigned short I;

	//Program Counter register
	unsigned short pc;

	//C8 screen has 2048 pixels (64 x 32). Create a Char array to hold each pixel
	unsigned char gfx[2048] = { 0 };

	//Delay Timer Register
	unsigned char delay_timer;

	//Sound Timer Register
	unsigned char sound_timer;

	//Stack Pointer Register. Points to topmost level of stack
	unsigned short stack_pointer;

	//Stack (memory stack) Register. C8 has a stack size of 16, each memory location is 16 bits (2 Bytes).
	unsigned short stack[16] = { 0 };

	//Random Device object for generating random numbers
	random_device randDevice;

public:

	//Member Variables
	
	int key[16]; //The 16 C8 Keys
	
	//Member Functions:

	void initialize(); //Initialize CPU registers and memory once

	void loadGame(string); //Load an external file (ROM) in to memory array
	
	void emulateCycle(); //Emulate one single cycle of CPU (Fetch, Decode, Execute)

	void drawPixels(); //Sets / plots the pixels to be rendered by renderPixels()

	void decreaseTimers(); //Decrements delay_timer and sound_timer
	
	//Test functions:

	void testEmulateCycle(); //Test emulate cycle, make sure it switches opcodes properly

	void initializeTest(); //Set up / initialize CPU registers and memory for testEmulateCycle

	void displayTest(); // Test the display opcode using the font-set

	void displayTest2(); // Print pixels (quads) at the four corners of the screen

	void inputTest(); // Test the input
};

