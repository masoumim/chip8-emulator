/*
Chip-8 Emulator
Author: Mark Masoumi
E-mail: masoumi.mark@gmail.com
Date: April 6 2020
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <GL/freeglut.h>
#include "Chip8.h"

using namespace std;

//debug variables
int cycleCount = 0;

//The Font Set of Numbers 0-9 and Hex digits A-F
const unsigned char fontSet[80] = {
0xF0, 0x90, 0x90, 0x90, 0xF0, //0
0x20, 0x60, 0x20, 0x20, 0x70, //1
0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
0x90, 0x90, 0xF0, 0x10, 0x10, //4
0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
0xF0, 0x10, 0x20, 0x40, 0x40, //7
0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
0xF0, 0x90, 0xF0, 0x90, 0x90, //A
0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
0xF0, 0x80, 0x80, 0x80, 0xF0, //C
0xE0, 0x90, 0x90, 0x90, 0xE0, //D
0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
0xF0, 0x80, 0xF0, 0x80, 0x80, //F
};


void chip8::initialize()
{
	//Initialize variables
	I = 0;
	pc = 512;
	delay_timer = 0;
	sound_timer = 0;
	stack_pointer = 0;
	V[0xF] = 0;

	//Clear memory array
	for (int i = 0; i < 4096; i++) {
		memory[i] = 0;
	}

	//Clear register array
	for (int i = 0; i < 16; i++) {
		V[i] = 0;
	}

	//Clear graphics array
	for (int i = 0; i < 2048; i++) {
		gfx[i] = 0;
	}

	//Clear stack array
	for (int i = 0; i < 16; i++) {
		stack[i] = 0;
	}

	//Load the font set in to the memory array starting at [0]
	for (int i = 0; i < 80; i++) {
		memory[i] = fontSet[i];
	}
}


void chip8::loadGame(string rom) {
	int memIndex = 512; //The index position in the memory array starting at 512 (0x200)

	//Load rom data in to memory array starting at 0x200 (512) to 0xFFF (4095)
	std::ifstream inputFile;
	inputFile.open(rom, std::ios::in | std::ios::binary);
	char ch;

	//Read each char in the ROM file and put it in memory array
	while (inputFile.get(ch)) {

		if (!inputFile.eof()) {
			memory[memIndex] = ch;
			memIndex++;
		}
	}

	inputFile.close();
}


void chip8::emulateCycle()
{
	//FETCH OpCode from memory. Each element in memory array stores 1 Byte (half an opcode), so two sequential elements must be combined to form one 2 Byte Opcode. The bitwise OR operator combines them.
	opcode = memory[pc] << 8 | memory[pc + 1];

	//DECODE & EXECUTE the Opcode. Find out which opcode it is by the first four bits (by using the Bitwise AND operator).
	switch (opcode & 0xF000) {

	case 0xA000: //ANNN: Sets I (current instruction register) to the address NNN.
		I = opcode & 0x0FFF;
		pc += 2; //Increment program counter by 2 (one would only be half an opcode).
		break;
	case 0xB000: //BNNN: The program counter is set to nnn plus the value of V0.

		pc = (opcode & 0x0FFF) + V[0];

		break;
	case 0xC000: //CXKK - Set Vx = random byte AND kk
	{
		//Generate a random number between 0 and 255
		uniform_int_distribution<int> distribution(0, 255);
		int randNum = distribution(randDevice);

		//V[x] stores result of bitwise AND between random number and value 'kk'
		V[(opcode & 0x0F00) >> 8] = randNum & (opcode & 0x00FF);

		pc += 2;
	}
	break;
	case 0xD000: //DXYN - DRW Vx, Vy, nibble
	{
		bool pixelFlipped = false;

		//Read N-bytes from memory array starting at address stored in I. Display sprites at coordinates (Vx, Vy)
		unsigned char xCoord = V[(opcode & 0x0F00) >> 8]; //x coord
		unsigned char yCoord = V[(opcode & 0x00F0) >> 4]; //y coord

		//Get newSprite
		for (int i = 0; i < (opcode & 0x000F); i++) {

			unsigned char newSprite = memory[I + i]; //sprite-byte in memory array
			unsigned char compareByte = 0x80; //10000000 in binary

			//Set gfx values using newSprite
			for (int k = 0; k < 8; k++) {

				if ((compareByte & newSprite) == compareByte) {

					if (gfx[(((xCoord + k) % 64) + (((yCoord + i) % 32) * 64))] == 1) {

						pixelFlipped = true;
					}

					gfx[(((xCoord + k) % 64) + (((yCoord + i) % 32) * 64))] ^= 1;
				}
				else {

					gfx[(((xCoord + k) % 64) + (((yCoord + i) % 32) * 64))] ^= 0;
				}

				compareByte /= 2; //Decrease comparison byte to compare the next less signigficant bit
			}
		}

		//Set V[F]
		if (pixelFlipped == true) {
			V[0xF] = 1;
		}
		else {
			V[0xF] = 0;
		}

		//Increment pc
		pc += 2;
	}
	break;
	case 0xE000:
		switch (opcode & 0x000F) {
		case 0x000E: //Ex9E - Skip next instruction if key with the value of Vx is pressed
		{
			if (key[V[(opcode & 0x0F00) >> 8]] == 1) {
				pc += 4;
			}
			else {
				pc += 2;
			}
		}
		break;
		case 0x0001: //ExA1 - Skip next instruction if key with the value of Vx is not pressed.

			if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;
		}
		break;
	case 0x0000:
		switch (opcode & 0x000F) {
		case 0x0000: //0x00E0 - Clears the screen

			for (int i = 0; i < 2048; i++) {
				gfx[i] = 0;
			}

			pc += 2; //Increment program counter by 2 (one would only be half an opcode).
			break;
		case 0x000E: //0x000E - Returns from subroutine

			stack_pointer--;
			pc = stack[stack_pointer]; //Set program counter to address at top of the stack
			pc += 2;
			break;
		}
		break;
	case 0xF000:
		switch (opcode & 0x000F) {

		case 0x0007: //Fx07 - Set Vx = delay timer value.
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: //FX0A - Wait for a key press, store the value of the key in Vx
		{
			for (int i = 0; i < 16; i++) {
				if (key[i] == 1) {

					V[(opcode & 0x0F00) >> 8] = i;

					pc += 2;
				}
			}
		}
		break;
		case 0x0008: //0xFX18 - Set sound timer = Vx
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E: //0xFX1E - Set I = I + Vx

			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF set to 1 if range overflow (I+VX>0xFFF), otherwise its set to 0.
				V[0xF] = 1;
			else
				V[0xF] = 0;

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0009: //0xFX29 - Set I = location of sprite for digit Vx
			I = (V[(opcode & 0x0F00) >> 8]) * 5;
			pc += 2;
			break;

		case 0x0003: //0xFX33 - Store BCD representation of Vx in memory locations I, I+1, and I+2
			memory[I] = (V[(opcode & 0x0F00) >> 8]) / 100; //Hundreds Digit
			memory[I + 1] = ((V[(opcode & 0x0F00) >> 8]) / 10) % 10; //Tens Digit
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8]) % 10; //Ones Digit
			pc += 2;
			break;
		case 0x0005:
			switch (opcode & 0x00F0) {
			case 0x0010:
				//0xFX15 - Set delay timer = Vx
				delay_timer = V[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			case 0x0050: //0xFX55 - Store registers V0 through Vx in memory starting at location I
				for (int j = 0; j <= ((opcode & 0x0F00) >> 8); j++) {
					memory[I + j] = V[j];
				}
				pc += 2;
				break;

			case 0x0060: //0xFX65 - The interpreter reads values from memory starting at location I into registers V0 through Vx
				for (int j = 0; j <= ((opcode & 0x0F00) >> 8); j++) {
					V[j] = memory[I + j];
				}
				pc += 2;
				break;
			}
			break;
		}
		break;
	case 0x1000: //0x1NNN - Jumps to address NNN
		pc = opcode & 0x0FFF; //Set the program counter to NNN
		break;
	case 0x2000: //0x2NNN - Calls a subroutine at NNN

		stack[stack_pointer] = pc;
		stack_pointer++;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000: //0x3xkk - Skip next instruction if Vx = kk
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x4000: //0x4xkk - Skip next instruction if Vx != kk
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x5000: //0x5xy0 - Skip next instruction if Vx = Vy
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	case 0x6000: //0x6xkk - The interpreter puts the value kk into register Vx
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;
	case 0x7000: //0x7xkk - Adds the value kk to the value of register Vx, then stores the result in Vx
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000: //0x8xy0 - Stores the value of register Vy in register Vx
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0001: //0x8xy1 - Set Vx = Vx OR Vy 
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;
		case 0x002: //0x8xy2 - Set Vx = Vx AND Vy
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;
		case 0x0003: //0x8xy3 - Set Vx = Vx XOR Vy
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;
		case 0x0004: //0x8xy4 - Set Vx = Vx + Vy, set VF = carry

			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]);

			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			pc += 2;
			break;
		case 0x0005: //0x8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.

			//Check if V[x] is greater than V[y]
			if ((V[(opcode & 0x0F00) >> 8]) > (V[(opcode & 0x00F0) >> 4])) {
				//If it is, set V[F] to 1
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8]) - (V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;
		case 0x0006: //0x8xy6 - Set Vx = Vx SHR 1

			//Check if the least significant bit of V[x] is 1
			if (V[(opcode & 0x0F00) >> 8] & 1 == 1) {
				V[0xF] = 1;
			}
			else {
				V[0XF] = 0;
			}

			//Bit-Shift V[x] right by 1
			V[(opcode & 0x0F00) >> 8] >>= 1;

			pc += 2;

			break;
		case 0x0007: //0x8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.

			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8]);
			pc += 2;
			break;
		case 0x000E: //0x8xyE - Set Vx = Vx SHL 1

			//V[F] is set to the Most Significant Bit of V[x]. If V[x] is less than 8 bits, the MSB is 0.
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;

			//Bit-shift V[x] left by 1
			V[(opcode & 0x0F00) >> 8] <<= 1;

			pc += 2;
			break;
		}
		break;
	case 0x9000: //9xy0 - Skip next instruction if Vx != Vy

		if ((V[(opcode & 0x0F00) >> 8]) != (V[(opcode & 0x00F0) >> 4])) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

	default: cout << "Unknown Opcode: " << opcode;
	}

	cycleCount++;
}


void chip8::decreaseTimers() {

	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
		--sound_timer;
}


void chip8::drawPixels() {
	glBegin(GL_QUADS);

	//Set Pixel color
	glColor3f(0, 1, 0);

	int k = 0;

	for (int i = 0; i < 32; i++) {

		for (int j = 0; j < 64; j++) {

			if (gfx[k] == 1) {

				glVertex2i(j, i); // top left
				glVertex2i(j + 1, i); // top right
				glVertex2i(j + 1, i + 1); //bottom right
				glVertex2i(j, i + 1); //bottom left
				k++;
			}
			else {
				k++;
			}
		}
	}
	glEnd();
}


//#########################################################################################################
//											TEST FUNCTIONS
//#########################################################################################################

void chip8::initializeTest()
{
	pc = 0; //Initialize program counter to 0
	I = 1; //Set Index Register (Memory Address Register) to 1
	stack_pointer = 1; //Set stack pointer to 1
	delay_timer = 5;
	sound_timer = 5;

	V[0xF] = 0; //Carry Flag
	V[0xA] = 0x00BC; //Set V at index 0xA to 0x00BC for testing opcode 0x3xkk
	V[0xB] = 0x00BC; //Set V at index 0xB to 0x00BC for testing opcode 0x5xy0
	V[0x1] = 0x0001; //Set V at index 0 to 0x0001 for testing opcode 0x7xkk
}


void chip8::testEmulateCycle() {

	std::cout << "Testing emulateCycle()" << endl;
	cout << "\n";

	//Add ANNN opcode to memory array
	memory[0] = 0xA0;
	memory[1] = 0x00;

	cout << "Attmepting to run opcode: ANNN" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();
	cout << "I = " << I << endl;

	//Add 0x00E0 opcode to memory array
	memory[2] = 0x00;
	memory[3] = 0xE0;

	cout << "\nAttmepting to run opcode: 0x00E0" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x000E opcode to memory array
	memory[4] = 0x00;
	memory[5] = 0x0E;

	cout << "\nAttmepting to run opcode: 0x000E" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x1NNN opcode to memory array
	memory[6] = 0x1A;
	memory[7] = 0xAA;
	pc = 6;

	cout << "\nAttmepting to run opcode: 0x1NNN" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x2NNN opcode to memory array
	memory[8] = 0x2A;
	memory[9] = 0xBC;
	pc = 8;

	cout << "\nAttmepting to run opcode: 0x2NNN" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x3xkk opcode to memory array
	memory[10] = 0x3A;
	memory[11] = 0xBC;
	pc = 10;

	cout << "\nAttmepting to run opcode: 0x3xkk" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x4xkk opcode to memory array
	memory[12] = 0x4A;
	memory[13] = 0xAA;
	pc = 12;

	cout << "\nAttmepting to run opcode: 0x4xkk" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x5xy0 opcode to memory array
	memory[14] = 0x5A;
	memory[15] = 0xB0;
	pc = 14;

	cout << "\nAttempting to run opcode: 0x5xy0" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();

	//Add 0x6xkk opcode to memory array
	memory[16] = 0x60;
	memory[17] = 0xAA;
	pc = 16;

	cout << "\nAttempting to run opcode: 0x6xkk" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();
	cout << "Value at V[" << ((opcode & 0x0F00) >> 8) << "] = " << hex << (int)V[(opcode & 0x0F00) >> 8] << endl;

	//Add 0x7xkk opcode to memory array
	memory[18] = 0x71;
	memory[19] = 0x01;
	pc = 18;

	cout << "\nAttempting to run opcode: 0x7xkk" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[1] before: " << hex << (int)V[1] << endl;
	cout << "Adding: 1 to " << hex << (int)V[1] << endl;
	emulateCycle();
	cout << "Value of V[" << ((opcode & 0x0F00) >> 8) << "] after: " << hex << (int)V[(opcode & 0x0F00) >> 8] << endl;

	//Add 0x8xy0 opcode to memory array
	memory[20] = 0x80;
	memory[21] = 0x10;
	pc = 20;

	cout << "\nAttempting to run opcode: 0x8xy0" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[1] = " << (int)V[1] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;

	//Add 0x8xy1 opcode to memory array
	memory[22] = 0x80;
	memory[23] = 0xA1;
	pc = 22;

	cout << "\nAttempting to run opcode: 0x8xy1" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[0xA] = " << (int)V[0xA] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;

	//Add 0x8xy2 opcode to memory array
	memory[24] = 0x80;
	memory[25] = 0xA2;
	pc = 24;

	cout << "\nAttempting to run opcode: 0x8xy2" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[0xA] = " << (int)V[0xA] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;

	//Add 0x8xy3 opcode to memory array
	memory[26] = 0x80;
	memory[27] = 0x23;
	pc = 26;


	cout << "\nAttempting to run opcode: 0x8xy3" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[2] = " << (int)V[2] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;


	//Add 0x8xy4 opcode to memory array
	memory[28] = 0x80;
	memory[29] = 0x14;
	pc = 28;

	V[0] = 0xFF;
	V[1] = 0x1;

	cout << "\nAttempting to run opcode: 0x8xy4" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[1] = " << (int)V[1] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;
	cout << "Carry Flag V[F] = " << (int)V[0xF] << endl;


	//Add 0x8xy5 opcode to memory array
	memory[30] = 0x80;
	memory[31] = 0x15;
	pc = 30;

	V[0] = 0xa;
	V[1] = 0x5;

	cout << "\nAttempting to run opcode: 0x8xy5" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[1] = " << (int)V[1] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;
	cout << "V[F] = " << (int)V[0xF] << endl;


	//Add 8xy6 opcode to memory array
	memory[32] = 0x80;
	memory[33] = 0x16;
	pc = 32;

	V[0] = 0xFF;
	V[0xF] = 0;

	cout << "\nAttempting to run opcode: 0x8xy6" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[F] before = " << (int)V[0xF] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;
	cout << "Value of V[F] after = " << (int)V[0xF] << endl;


	//Add 8xy7 opcode to memory array
	memory[34] = 0x80;
	memory[35] = 0x17;
	pc = 34;

	V[0] = 0x3;
	V[1] = 0xa;

	cout << "\nAttempting to run opcode: 0x8xy7" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[1] = " << (int)V[1] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;
	cout << "Value of V[F] after = " << (int)V[0xF] << endl;

	//Add 8xyE opcode to memory array
	memory[36] = 0x80;
	memory[37] = 0x1E;
	pc = 36;

	V[0] = 0xFF;
	V[0xF] = 0;

	cout << "\nAttempting to run opcode: 0x8xyE" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] before = " << (int)V[0] << endl;
	cout << "Value of V[F] before = " << (int)V[0xF] << endl;
	emulateCycle();
	cout << "Value of V[0] after = " << (int)V[(opcode & 0x0F00) >> 8] << endl;
	cout << "Value of V[F] after = " << (int)V[0xF] << endl;


	//Add 9xy0 opcode to memory array
	memory[38] = 0x90;
	memory[39] = 0x10;
	pc = 38;

	V[0] = 0xF;
	V[1] = 0xE;


	cout << "\nAttempting to run opcode: 0x9xy0" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] = " << (int)V[0] << endl;
	cout << "Value of V[1] = " << (int)V[1] << endl;
	cout << "Value of Program Counter =  " << dec << pc << endl;
	emulateCycle();
	cout << "Value of Program Counter =  " << dec << pc << endl;


	//Add BNNN opcode to memory array
	memory[40] = 0xBA;
	memory[41] = 0xAA;
	pc = 40;

	V[0] = 0x1;

	cout << "\nAttempting to run opcode: 0xBNNN" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[0] = " << (int)V[0] << endl;
	cout << "Value of Program Counter before =  " << dec << pc << endl;
	emulateCycle();
	cout << "Value of Program Counter after =  " << dec << pc << endl;


	//Add Cxkk opcode to memory array
	memory[42] = 0xC0;
	memory[43] = 0x0B;
	pc = 42;

	V[0] = 0;

	cout << "\nAttempting to run opcode: 0xCXKK" << endl;
	cout << "---------------------------------" << endl;
	cout << "Value of V[x] = " << (int)V[0] << endl;
	cout << "Vaue of kk = " << 0x0B << endl;
	emulateCycle();
	cout << "Value of V[x] after = " << (int)V[0] << endl;


	//Add Dxyn opcode to memory array
	memory[44] = 0xD0;
	memory[45] = 0x0A;
	pc = 44;

	//0
	memory[0] = 0xF0;
	memory[1] = 0x90;
	memory[2] = 0x90;
	memory[3] = 0x90;
	memory[4] = 0xF0;

	//1
	memory[5] = 0x20;
	memory[6] = 0x60;
	memory[7] = 0x20;
	memory[8] = 0x20;
	memory[9] = 0x70;

	I = 0;
	V[0] = 0;

	cout << "\nAttempting to run opcode: 0xDXYN" << endl;
	cout << "---------------------------------" << endl;
	emulateCycle();


	//Add Ex9E opcode to memory array
	memory[46] = 0xE1;
	memory[47] = 0x9E;
	pc = 46;

	V[1] = 65; //65 is ASCII for 'A'

	cout << "\nAttempting to run opcode: 0xEX9E" << endl;
	cout << "---------------------------------" << endl;
	cout << "Program Counter before: " << pc << endl;
	emulateCycle();
	cout << "Program Counter after: " << pc << endl;


	//Add ExA1 opcode to memory array
	memory[48] = 0xE1;
	memory[49] = 0xA1;
	pc = 48;

	V[1] = 65; //65 is ASCII for 'A'

	cout << "\nAttempting to run opcode: 0xEXA1" << endl;
	cout << "---------------------------------" << endl;
	cout << "Program Counter before: " << pc << endl;
	emulateCycle();
	cout << "Program Counter after: " << pc << endl;


	//Add FX07 opcode to memory array
	memory[50] = 0xF1;
	memory[51] = 0x07;
	pc = 50;

	V[1] = 0;

	cout << "\nAttempting to run opcode: 0xFX07" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] before = " << (int)V[1] << endl;
	cout << "Delay Timer = " << (int)delay_timer << endl;
	emulateCycle();
	cout << "V[x] after = " << (int)V[1] << endl;


	//Add FX0A opcode to memory array
	memory[52] = 0xF1;
	memory[53] = 0x0A;
	pc = 52;

	V[1] = 0;

	cout << "\nAttempting to run opcode: 0xFX0A" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] before = " << (int)V[1] << endl;
	emulateCycle();
	cout << "V[x] before = " << (int)V[1] << endl;


	//Add FX15 opcode to the memory array
	memory[54] = 0xF1;
	memory[55] = 0x15;
	pc = 54;

	V[1] = 10;

	cout << "\nAttempting to run opcode: 0xFX15" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] = " << (int)V[1] << endl;
	cout << "Delay Timer before = " << (int)delay_timer << endl;
	emulateCycle();
	cout << "Delay Timer after = " << (int)delay_timer << endl;


	//Add FX18 opcode to memory array
	memory[56] = 0xF1;
	memory[57] = 0x18;
	pc = 56;

	V[1] = 9;

	cout << "\nAttempting to run opcode: 0xFX18" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] = " << (int)V[1] << endl;
	cout << "Sound Timer before = " << (int)sound_timer << endl;
	emulateCycle();
	cout << "Sound Timer after = " << (int)sound_timer << endl;

	//Add FX1E opcode to memory array
	memory[58] = 0xF1;
	memory[59] = 0x1E;
	pc = 58;

	V[1] = 10;

	cout << "\nAttempting to run opcode: 0xFX1E" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] = " << (int)V[1] << endl;
	cout << "Index Register before = " << I << endl;
	emulateCycle();
	cout << "Index Register after = " << I << endl;

	//Add FX29 opcode to memory array
	memory[60] = 0xF1;
	memory[61] = 0x29;
	pc = 60;

	V[1] = 0xa;

	cout << "\nAttempting to run opcode: 0xFX29" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] = " << (int)V[1] << endl;
	cout << "Index Register before = " << I << endl;
	emulateCycle();
	cout << "Index Register after = " << dec << I << endl;


	//Add FX33 opcode to memory array
	memory[62] = 0xF1;
	memory[63] = 0x33;
	pc = 62;

	V[1] = 123;
	I = 0;

	cout << "\nAttempting to run opcode: 0xFX33" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[x] = " << (int)V[1] << endl;
	emulateCycle();
	cout << "Hundreds Digit: " << (int)memory[I] << endl;
	cout << "Tens Digit: " << (int)memory[I + 1] << endl;
	cout << "Ones Digit: " << (int)memory[I + 2] << endl;


	//Add FX55 opcode to memory
	memory[64] = 0xF5;
	memory[65] = 0x55;
	pc = 64;

	I = 0;
	V[0] = 25;
	V[1] = 24;
	V[2] = 23;
	V[3] = 22;
	V[4] = 21;
	V[5] = 20;

	cout << "\nAttempting to run opcode: 0xFX55" << endl;
	cout << "---------------------------------" << endl;
	cout << "Memory[] before:" << endl;
	cout << dec << (int)memory[0] << endl;
	cout << (int)memory[1] << endl;
	cout << (int)memory[2] << endl;
	cout << (int)memory[3] << endl;
	cout << (int)memory[4] << endl;
	cout << (int)memory[5] << endl;
	emulateCycle();
	cout << "Memory[] after:" << endl;
	cout << dec << (int)memory[0] << endl;
	cout << (int)memory[1] << endl;
	cout << (int)memory[2] << endl;
	cout << (int)memory[3] << endl;
	cout << (int)memory[4] << endl;
	cout << (int)memory[5] << endl;


	//Add FX65 opcode to memory
	memory[66] = 0xF5;
	memory[67] = 0x65;
	pc = 66;

	I = 0;
	memory[0] = 55;
	memory[1] = 54;
	memory[2] = 53;
	memory[3] = 52;
	memory[4] = 51;
	memory[5] = 50;

	cout << "\nAttempting to run opcode: 0xFX65" << endl;
	cout << "---------------------------------" << endl;
	cout << "V[] before:" << endl;
	cout << dec << (int)V[0] << endl;
	cout << (int)V[1] << endl;
	cout << (int)V[2] << endl;
	cout << (int)V[3] << endl;
	cout << (int)V[4] << endl;
	cout << (int)V[5] << endl;
	emulateCycle();
	cout << "V[] after:" << endl;
	cout << dec << (int)memory[0] << endl;
	cout << (int)V[1] << endl;
	cout << (int)V[2] << endl;
	cout << (int)V[3] << endl;
	cout << (int)V[4] << endl;
	cout << (int)V[5] << endl;
}


void chip8::displayTest() {

	// Dxyn - DRW Vx, Vy, nibble
	// Display n - byte sprite starting at memory location I at(Vx, Vy), set VF = collision.

	//Add font set to memory array
	for (int i = 0; i < 80; i++) {


		memory[i] = fontSet[i];
	}

	//Initialize variables
	I = 0;
	pc = 81;
	V[0] = 0;
	V[1] = 0;
	int memIndex_a = 81;
	int memIndex_b = 82;


	//Print the numbers (0-9)
	for (int j = 0; j < 10; j++) {

		memory[memIndex_a] = 0xD0;
		memory[memIndex_b] = 0x15;

		emulateCycle();

		memIndex_a += 2;
		memIndex_b += 2;
		I += 5;
		pc += 2;
		V[0] += 5;
	}

	//Set x,y coordinates to next row
	V[0] = 0;
	V[1] = 6;

	//Print the hex letters (A-F)
	for (int k = 0; k < 6; k++) {

		memory[memIndex_a] = 0xD0;
		memory[memIndex_b] = 0x15;

		emulateCycle();

		memIndex_a += 2;
		memIndex_b += 2;
		I += 5;
		pc += 2;
		V[0] += 5;
	}

}


void chip8::displayTest2() {

	//NOTE: Disable the screen clear at the start of display opcode (DXYN) to display all 4 pixels!

	//Initialize variables
	I = 0;
	pc = 1;
	memory[0] = 0x80; //10000000

	//Print pixels (quads) at four corners of screen

	//Top Left
	V[0] = 1;
	V[1] = 1;

	memory[1] = 0xD0;
	memory[2] = 0x11;
	emulateCycle();

	//Top Right
	V[0] = 64;
	V[1] = 1;

	memory[3] = 0xD0;
	memory[4] = 0x11;
	emulateCycle();

	//Bottom Left
	V[0] = 1;
	V[1] = 32;

	memory[5] = 0xD0;
	memory[6] = 0x11;
	emulateCycle();

	//Bottom Right
	V[0] = 64;
	V[1] = 32;

	memory[7] = 0xD0;
	memory[8] = 0x11;
	emulateCycle();
}


void chip8::inputTest() {
	pc = 0;
	int dummy = 0;
	while (dummy == 0) {
		memory[0] = 0xF0;
		memory[1] = 0x0A;
		emulateCycle();
	}
}