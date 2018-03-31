#include "Cpu.h"
#include <iostream>
#include <fstream>
#include <ctime>

Chip8::Chip8()
{
	// Clear registers
	opcode = 0;
	I = 0;
	sp = 0;
	pc = 0x200;		// ROM will be loaded at this location in memory
	
	drawFlag = true;

	// Clear data storages
	for (int i = 0; i < NUM_REGISTERS; ++i)
		V[i] = 0;

	for (int i = 0; i < NUM_KEYS; ++i)
		key[i] = 0;

	for (int i = 0; i < STACK_SIZE; ++i)
		stack[i] = 0;

	for (int i = 0; i < MEMORY_SIZE; ++i)
		memory[i] = 0;

	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
		gfx[i] = 0;

	// @TODO: Load fontset ??!

	// Clear timers
	soundTimer = 0; // @TODO: Correct ?!
	delayTimer = 0;

	srand(time(NULL));
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
			std::cout << "Error loading ROM: Not enough space in memory!" << std::endl;
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
	opcode = FetchOpcode();

	// Decode and execute opcode
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // Display, clears the screen
			for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
				gfx[i] = 0;

			drawFlag = true;
			UpdatePC();
			break;

		case 0x000E: // Flow, returns from subroutine
			--sp;
			pc = stack[sp];
			UpdatePC(); // @TODO: Correct ?!
			break;

		default:
			std::cout << "Error (decode): Bad opcode (0x0000): " << opcode << std::endl;
		}
		break;

	case 0x1000: // Flow, goto NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: // Flow, calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0xFFF;
		break;

	case 0x3000: // Cond [0x3xNN], if (Vx == NN), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] == opcode & 0x00FF) ? pc += 4 : UpdatePC();
		break;

	case 0x4000: // Cond [0x4xNN], if (Vx != NN), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] != opcode & 0x00FF) ? pc += 4 : UpdatePC();
		break;

	case 0x5000: // Cond, if (Vx == Vy), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 8]) ? pc += 4 : UpdatePC();
		break;

	case 0x6000: // Const [0x6xNN], sets Vx to NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		UpdatePC();
		break;

	case 0x7000: // Const [0x7xNN], adds NN to Vx
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		UpdatePC();
		break;

	case 0x8000: // Instructions group, [0x8xyZ], Z=0...7 and E
		switch (opcode & 0x000F)
		{
		case 0x0000: // Assign, Vx = Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			break;

		case 0x0001: // BitOp, Vx = Vx | Vy
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			break;

		case 0x0002: // BitOp, Vx = Vx & Vy
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			break;

		case 0x0003: // BitOp, Vx = Vx ^ Vy
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			break;

		case 0x0004: // Math, Vx += Vy
			if (V[(opcode & 0x0F00) >> 4] > 0xFF - V[(opcode & 0x0F00) >> 8])
				V[CARRY_FLAG] = 1;
			else
				V[CARRY_FLAG] = 0;

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			break;

		case 0x0005: // Math, Vx -= Vy
			if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x0F00) >> 4])
				V[CARRY_FLAG] = 0;
			else
				V[CARRY_FLAG] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			break;

		case 0x0006: // BitOp, Shift Vy>>1 and copy result to Vx. Set VF to least sig.bit of VY before shift
			V[CARRY_FLAG] = V[(opcode & 0x00F0) >> 4] & 0b00000001; // @TODO: Correct ?!!
			V[(opcode & 0x00F0) >> 4] >>= 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			break;

		case 0x0007: // Math, Vx = Vy - Vx
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x0F00) >> 4])
				V[CARRY_FLAG] = 0;
			else
				V[CARRY_FLAG] = 1;

			V[V[(opcode & 0x0F00) >> 8]] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			break;

		case 0x000E: // BitOp, Vx=Vy=Vy<<1
			V[CARRY_FLAG] = V[(opcode & 0x00F0) >> 4] & 0b00000001; // @TODO: Correct ?!!
			V[(opcode & 0x00F0) >> 4] <<= 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			break;

		default:
			std::cout << "Error (decode): Bad opcode (0x8000): " << opcode << std::endl;
		}
		break;

	case 0x9000: // Cond, if (Vx != Vy) skips nexts instruction
		(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) ? pc += 4 : UpdatePC();
		break;

	case 0xA000: // Mem, sets I to address NNN
		I = opcode & 0x0FFF;
		UpdatePC();
		break;

	case 0xB000: // Flow [0xBNNN], pc = v0 + NNN
		pc = V[0] + (opcode & 0x0FFF);
		break;

	case 0xC000: // Rand [0xCxNN], Vx = rand() % 255 & NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
		UpdatePC();
		break;

	case 0xD000: // Disp, draw(Vx, Vy, N)

		break;

	case 0xE000:
		switch (opcode & 0x000F)
		{
		case 0x000E: // KeyOp, if (key() == Vx)

			break;

		case 0x0001: // KeyOp, if (key() != Vx)

			break;

		default:
			std::cout << "Error (decode): Bad opcode (0xE000): " << opcode << std::endl;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // Timer, Vx = get_delay()

			break;

		case 0x000A: // KeyOp, Vx = get_key()

			break;

		case 0x0015: // Timer, delay_timer(Vx)
			delayTimer = V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			break;

		case 0x0018: // Sound, sound_timer(Vx)
			soundTimer = V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			break;

		case 0x001E: // Mem, I += Vx
			I += V[(opcode & 0x0F00) >> 8]; // @TODO: Check overflow ?!
			UpdatePC();
			break;

		case 0x0029: // Mem, I = sprite_addr[Vx]

			break;

		case 0x0033: // Bcd
			
			break;

		case 0x0055: // Mem, reg_dump(Vx, &I)
			
			break;

		case 0x0065: // Mem, reg_load(Vx, &I)

			break;

		default:
			std::cout << "Error (decode): Bad opcode (0xF000): " << opcode << std::endl;
		}
		break;

	default:
		std::cout << "Error (Decode): Unknown opcode " << opcode << "." << std::endl;
	}

	UpdateTimers();
}

/* Get next opcode from memory. Big-endian. */
unsigned short Chip8::FetchOpcode()
{
	unsigned char highByte = memory[pc];
	unsigned char lowByte  = memory[pc + 1];

	return highByte << 8 | lowByte;
}

/* Decreasing timers until they reach 0. */
void Chip8::UpdateTimers()
{
	// @TODO: Set timers to start value after they become negative numbers
	if (delayTimer > 0)
		--delayTimer;
	
	if (soundTimer > 0)
		--soundTimer;
}

/* Increase program counter by 2 bytes. */
void Chip8::UpdatePC()
{
	pc += 2;
}