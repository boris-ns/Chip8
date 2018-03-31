#pragma once

#include <string>

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_SIZE 16
#define NUM_KEYS 16

class Chip8
{
public:
	Chip8();
	~Chip8();

	void MainLoop();
	void EmulateCycle();
	void LoadROM(const std::string& romPath);

	unsigned short FetchOpcode();
	unsigned short DecodeOpcode(unsigned short opcode);
	unsigned short ExecuteOpcode(unsigned short opcode);
	void UpdateTimers();

private:

	// Registers
	unsigned short opcode;								// current opcode
	unsigned char V[NUM_REGISTERS];						// registers
	unsigned short I;									// index register
	unsigned short pc;									// program counter register
	unsigned char sp;									// stack pointer
	
	// Timers
	unsigned char delayTimer;
	unsigned char soundTimer;

	// Data storage
	unsigned char memory[MEMORY_SIZE];					// 4K memory
	unsigned char gfx[SCREEN_WIDTH * SCREEN_HEIGHT];	// screen
	unsigned char stack[STACK_SIZE];					// stack for jump instructions and function calls
	unsigned char key[NUM_KEYS];						// keyboard state
};
