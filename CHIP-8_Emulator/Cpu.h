#pragma once

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_SIZE 16
#define NUM_KEYS 16

class Chip8Cpu
{
public:
	Chip8Cpu();
	~Chip8Cpu();

	void EmulateCycle();

private:
	unsigned short opcode;								// current opcode
	unsigned char memory[MEMORY_SIZE];					// 4K memory
	unsigned char V[NUM_REGISTERS];						// registers
	unsigned short I;									// index register
	unsigned short pc;									// program counter register
	unsigned char gfx[SCREEN_WIDTH * SCREEN_HEIGHT];	// screen
	unsigned char delayTimer;
	unsigned char soundTimer;
	unsigned char stack[STACK_SIZE];					// stack for jump instructions and function calls
	unsigned char sp;									// stack pointer
	unsigned char key[NUM_KEYS];						// keyboard state
};
