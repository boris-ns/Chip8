#pragma once

#include <string>
#include "SFML/Graphics.hpp"

#define MEMORY_SIZE   4096
#define NUM_REGISTERS 16
#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32
#define STACK_SIZE    16
#define NUM_KEYS      16
#define FONTSET_SIZE  80
#define NUM_PIXELS    SCREEN_WIDTH * SCREEN_HEIGHT

class Chip8
{
public:
	Chip8();
	~Chip8();

	void MainLoop();
	void EmulateCycle();
	void LoadROM(const std::string& romPath);
	void Render(sf::RenderWindow& window);
	void HandleEvents(sf::RenderWindow& window);
	void Chip8::SwitchKeyState(sf::Keyboard::Key pressedKey, int state);

	unsigned short FetchOpcode();
	void DecodeExecute();
	void UpdateTimers();
	void UpdatePC();

private:

	bool drawFlag;
	static const unsigned char fontset[FONTSET_SIZE];
	const int CARRY_FLAG = NUM_REGISTERS - 1;
	sf::Uint8 screenImage[NUM_PIXELS * 4];				// contains RGBA values

	// Registers
	unsigned char sp;									// stack pointer
	unsigned char V[NUM_REGISTERS];						// registers
	unsigned short I;									// index register
	unsigned short pc;									// program counter register
	unsigned short opcode;								// current opcode
	
	// Timers
	unsigned char delayTimer;
	unsigned char soundTimer;

	// Data storage
	unsigned char gfx[NUM_PIXELS];	                    // screen
	unsigned char key[NUM_KEYS];						// keyboard state
	unsigned char memory[MEMORY_SIZE];					// 4K memory
	unsigned short stack[STACK_SIZE];					// stack for jump instructions and function calls
};

void Log(const std::string& message);