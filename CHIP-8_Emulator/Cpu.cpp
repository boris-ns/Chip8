#include "Cpu.h"
#include "SFML/Graphics.hpp"
#include <iostream>
#include <fstream>
#include <ctime>

const unsigned char Chip8::fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{
	// Clear registers
	opcode = 0;
	I = 0;
	sp = 0;
	pc = 0x200;		// ROM will be loaded at this location in memory
	
	drawFlag = true;

	// Prepare data storages
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

	for (int i = 80; i < FONTSET_SIZE; ++i)
		memory[i] = fontset[i];

	// Clear timers
	soundTimer = 0; 
	delayTimer = 0;

	srand(time(NULL));

	std::cout << "Chip8 initialized." << std::endl;
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

	std::cout << "ROM loaded successfully." << std::endl;
}

void Chip8::MainLoop()
{
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Chip8");

	while (window.isOpen())
	{
		//EmulateCycle();

		// Render
		if (drawFlag)
			Render(window);

		// UpdateKeys();
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
	}
}

/* Fill Uint8 array. This array is used to create sf::Image object which is going to be drawn. */
void Chip8::Render(sf::RenderWindow& window)
{
	for (int i = 0, j = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i, j += 4)
	{
		// Pixel is activated
		if (gfx[i] != 0)
		{
			screenImage[j]     = 0; // Red
			screenImage[j + 1] = 0; // Green
			screenImage[j + 2] = 0; // Blue
			screenImage[j + 3] = 1; // Alpha
		}
		else
		{
			screenImage[j]     = 255; // Red
			screenImage[j + 1] = 255; // Green
			screenImage[j + 2] = 255; // Blue
			screenImage[j + 3] =   1; // Alpha
		}
	}

	sf::Image image;
	image.create(SCREEN_WIDTH, SCREEN_HEIGHT, screenImage);

	sf::Texture texture;
	texture.loadFromImage(image);
	sf::Sprite sprite;
	sprite.setTexture(texture, true);

	window.clear();
	window.draw(sprite);
	window.display();
}

void Chip8::UpdateKeys()
{

}

void Chip8::EmulateCycle()
{
	opcode = FetchOpcode();
	DecodeExecute();
	UpdateTimers();
}

void Chip8::DecodeExecute()
{
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
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;
		V[CARRY_FLAG] = 0; // CF is set to 1 when there was a change of pixel. It's mechanism for collision detection

		for (int i = 0; i < height; ++i)
		{
			pixel = memory[I + i];
			for (int j = 0; j < 8; ++j)
			{
				// If pixel is activated
				if (pixel & (0x80 >> j) != 0)
				{
					// Check for collision
					if (gfx[x + j + (y + i) * SCREEN_WIDTH] == 1)
						V[CARRY_FLAG] = 1;

					// Activate/Deactivate pixel
					gfx[x + j + (y + i) * SCREEN_WIDTH] ^= 1;
				}
			}
		}

		drawFlag = true;
		UpdatePC();
	}
	break;

	case 0xE000:
		switch (opcode & 0x000F)
		{
		case 0x000E: // KeyOp, if (key() == Vx)
			(key[V[(opcode & 0x0F00) >> 8]] != 0) ? pc += 4 : UpdatePC();
			break;

		case 0x0001: // KeyOp, if (key() != Vx)
			(key[V[(opcode & 0x0F00) >> 8]] == 0) ? pc += 4 : UpdatePC();
			break;

		default:
			std::cout << "Error (decode): Bad opcode (0xE000): " << opcode << std::endl;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // Timer, Vx = get_delay()
			V[(opcode & 0x0F00) >> 8] = delayTimer;
			UpdatePC();
			break;

		case 0x000A: // KeyOp, Vx = get_key()
		{
			bool keyPressed = false;

			for (int i = 0; i < NUM_KEYS; ++i)
			{
				// Key is pressed
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i; // Set Vx to pressed key (i)
					keyPressed = true;
				}
			}

			// Go to next instruction only if key is pressed.
			// Mechanism for waiting a key press is to not update program counter.
			if (keyPressed)
				UpdatePC();
		}
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
			I = V[(opcode & 0x0F00) >> 8] * 5;
			UpdatePC();
			break;

		case 0x0033: // Bcd
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			UpdatePC();
			break;

		case 0x0055: // Mem, reg_dump(Vx, &I)
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
				memory[I++] = V[i];

			UpdatePC();
			break;

		case 0x0065: // Mem, reg_load(Vx, &I)
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
				V[i] = memory[I++];

			UpdatePC();
			break;

		default:
			std::cout << "Error (decode): Bad opcode (0xF000): " << opcode << std::endl;
		}
		break;

	default:
		std::cout << "Error (Decode): Unknown opcode " << opcode << "." << std::endl;
	}
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