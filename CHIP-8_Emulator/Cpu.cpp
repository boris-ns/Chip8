#include "Cpu.h"
#include <iostream>
#include <fstream>

Chip8::Chip8()
{
	// Clear registers
	opcode = 0;
	I = 0;
	sp = 0;
	pc = 0x200;		// ROM will be loaded at this location in memory
	
	// Clear data storages
	for (int i = 0; i < NUM_REGISTERS; ++i)
		V[i] = 0;

	for (int i = 0; i < NUM_KEYS; ++i)
		key[i] = 0;

	for (int i = 0; i < STACK_SIZE; ++i)
		stack[i] = 0;

	for (int i = 0; i < MEMORY_SIZE; ++i)
		memory[i] = 0;

	// @TODO: Load fontset ??!

	// Clear timers
	soundTimer = 60; // @TODO: Correct ?!
	delayTimer = 60;
}

Chip8::~Chip8()
{
}

void Chip8::LoadROM(const std::string& romPath)
{
	std::ifstream inputFile(romPath, std::ios_base::binary);

	// Start filling memory from location 512
	int memLoc = 512;
	while (!inputFile.eof())
	{
		if (memLoc > MEMORY_SIZE)
		{
			std::cout << "Error loading ROM: Not enough space in memory!";
			return;
		}

		unsigned char data = 0;
		inputFile.read((char*)&data, sizeof(unsigned char));
		
		memory[memLoc] = data;
		++memLoc;
	}

	inputFile.close();
}

void Chip8::MainLoop()
{

}

void Chip8::EmulateCycle()
{
	unsigned short opcode = FetchOpcode();

	// Decode Opcode

	// Execute Opcode

	// Update timers
	UpdateTimers();
}

unsigned short Chip8::FetchOpcode()
{
	unsigned char highByte = memory[pc];
	unsigned char lowByte  = memory[pc + 1];

	return highByte << 8 | lowByte;
}

unsigned short Chip8::DecodeOpcode(unsigned short opcode)
{

}

unsigned short Chip8::ExecuteOpcode(unsigned short opcode)
{

}

void Chip8::UpdateTimers()
{
	// @TODO: Set timers to start value after they become negative numbers
	if (delayTimer > 0)
		--delayTimer;
	
	if (soundTimer > 0)
		--soundTimer;
}