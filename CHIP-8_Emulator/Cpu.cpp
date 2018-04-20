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

	for (int i = 80, j = 0; j < FONTSET_SIZE; ++i, ++j)
		memory[i] = fontset[j];

	// Clear timers
	soundTimer = 0; 
	delayTimer = 0;

	srand(time(NULL));

	Log("Chip8 initialized.");
}

Chip8::~Chip8()
{
}

/* Method for loading ROM into Chip8 memory array. 
 * Writing in memory starts at location 512.
 * Also PC is set to 512 in constructor.*/
bool Chip8::LoadROM(const std::string& romPath)
{
	std::ifstream inputFile(romPath, std::ios_base::binary);

	// Start filling memory from location 512
	int memLoc = 512;
	while (!inputFile.eof())
	{
		if (memLoc > MEMORY_SIZE)
		{
			Log("Error loading ROM: Not enough space in memory!");
			return false;
		}

		unsigned char data = 0;
		inputFile.read((char*)&data, sizeof(unsigned char));
		
		memory[memLoc] = data;
		++memLoc;
	}

	inputFile.close();

	Log("ROM loaded successfully.");
	return true;
}

/* Main loop of emulator. Loop is active until user closes the window. */
void Chip8::MainLoop()
{
	unsigned int newWidth  = SCREEN_WIDTH  * MULTIPLIER;
	unsigned int newHeight = SCREEN_HEIGHT * MULTIPLIER;

	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Chip8");
	window.setSize(sf::Vector2u(newWidth, newHeight)); // @Hack: make window and rendering picture bigger w/out changing resolution in CPU
	//window.setFramerateLimit(180); // Real Chip8 works at 60Hz, this is SFML frame limiter
	
	while (window.isOpen())
	{
		EmulateCycle();

		if (drawFlag)
			Render(window);

		HandleEvents(window);
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
			screenImage[j]     = 255; // Red
			screenImage[j + 1] = 255; // Green
			screenImage[j + 2] = 255; // Blue
			screenImage[j + 3] = 255; // Alpha
		}
		else
		{
			screenImage[j]     = 0; // Red
			screenImage[j + 1] = 0; // Green
			screenImage[j + 2] = 0; // Blue
			screenImage[j + 3] = 255; // Alpha
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

/* Method for handling events. Right now events of interest
 * are closed, keypressed and keyreleased*/
void Chip8::HandleEvents(sf::RenderWindow& window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			break;

		case sf::Event::KeyPressed:
			SwitchKeyState(event.key.code, 1);
			break;

		case sf::Event::KeyReleased:
			SwitchKeyState(event.key.code, 0);
			break;

		default:
			Log("Warning (HandleEvent): There's no handler for that event.");
		}
	}
}

/* Switches keystate of key. */
void Chip8::SwitchKeyState(sf::Keyboard::Key pressedKey, int state)
{
	switch (pressedKey)
	{
	case sf::Keyboard::Num1:
		key[0] = state;
		break;
	case sf::Keyboard::Num2:
		key[1] = state;
		break;
	case sf::Keyboard::Num3:
		key[2] = state;
		break;
	case sf::Keyboard::Num4:
		key[3] = state;
		break;
	case sf::Keyboard::Q:
		key[4] = state;
		break;
	case sf::Keyboard::W:
		key[5] = state;
		break;
	case sf::Keyboard::E:
		key[6] = state;
		break;
	case sf::Keyboard::R:
		key[7] = state;
		break;
	case sf::Keyboard::A:
		key[8] = state;
		break;
	case sf::Keyboard::S:
		key[9] = state;
		break;
	case sf::Keyboard::D:
		key[10] = state;
		break;
	case sf::Keyboard::F:
		key[11] = state;
		break;
	case sf::Keyboard::Z:
		key[12] = state;
		break;
	case sf::Keyboard::X:
		key[13] = state;
		break;
	case sf::Keyboard::C:
		key[14] = state;
		break;
	case sf::Keyboard::V:
		key[15] = state;
		break;
	default:
		Log("Error (Keyboard): Wrong key code.");
	}
}

/* 1 cycle of emulation. Fetch opcode, decode, execute and update timers. */
void Chip8::EmulateCycle()
{
	opcode = FetchOpcode();
	DecodeExecute();
	UpdateTimers();
}

/* Decodes and executes Chip8 opcode */
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
			Log("[00E0] Display, clear the screen");
			break;

		case 0x000E: // Flow, returns from subroutine
			--sp;
			pc = stack[sp];
			UpdatePC(); // @TODO: Correct ?!
			Log("[00EE] Flow, return from subroutine");
			break;

		default:
			Log("Error (decode): Bad opcode (0x0000): " + opcode);
		}
		break;

	case 0x1000: // Flow, goto NNN
		pc = opcode & 0x0FFF;
		Log("[1NNN] Flow, jump to NNN");
		break;

	case 0x2000: // Flow, calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		Log("[2NNN] Flow, calls subroutine at NNN");
		break;

	case 0x3000: // Cond [0x3xNN], if (Vx == NN), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) ? pc += 4 : UpdatePC();
		Log("[3XNN] Cond, skips instr. if VX==NN");
		break;

	case 0x4000: // Cond [0x4xNN], if (Vx != NN), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) ? pc += 4 : UpdatePC();
		Log("[4XNN] Cond, skips instr. if VX!=NN");
		break;

	case 0x5000: // Cond, if (Vx == Vy), skips 1 instruction
		(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 8]) ? pc += 4 : UpdatePC();
		Log("[5XY0] Cond, skips instr. if VX==VY");
		break;

	case 0x6000: // Const [0x6xNN], sets Vx to NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		Log("[6XNN] Const, set Vx = NN");
		UpdatePC();
		break;

	case 0x7000: // Const [0x7xNN], adds NN to Vx
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		Log("[7XNN] Const, Vx += NN");
		UpdatePC();
		break;

	case 0x8000: // Instructions group, [0x8xyZ], Z=0...7 and E
		switch (opcode & 0x000F)
		{
		case 0x0000: // Assign, Vx = Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			Log("[8XY0] Assign, Vx = Vy");
			break;

		case 0x0001: // BitOp, Vx = Vx | Vy
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			Log("[8XY1] BitOp, Vx = Vx | Vy");
			break;

		case 0x0002: // BitOp, Vx = Vx & Vy
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			Log("[8XY2] BitOp, Vx = Vx & Vy");
			break;

		case 0x0003: // BitOp, Vx = Vx ^ Vy
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			Log("[8XY3] BitOp, Vx = Vx ^ Vy");
			break;

		case 0x0004: // Math, Vx += Vy
			if (V[(opcode & 0x0F00) >> 4] > 0xFF - V[(opcode & 0x0F00) >> 8])
				V[CARRY_FLAG] = 1;
			else
				V[CARRY_FLAG] = 0;

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			Log("[8XY4] Math, Vx += Vy w/CF");
			break;

		case 0x0005: // Math, Vx -= Vy
			if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x0F00) >> 4])
				V[CARRY_FLAG] = 0;
			else
				V[CARRY_FLAG] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x0F00) >> 4];
			UpdatePC();
			Log("[8XY5] Math, Vx -= Vy w/CF");
			break;

		case 0x0006: // BitOp, Shift Vy>>1 and copy result to Vx. Set VF to least sig.bit of VY before shift
			V[CARRY_FLAG] = V[(opcode & 0x00F0) >> 4] & 0b00000001; // @TODO: Correct ?!!
			V[(opcode & 0x00F0) >> 4] >>= 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			Log("[8XY6] BitOp, Vx=Vy=Vy>>1");
			break;

		case 0x0007: // Math, Vx = Vy - Vx
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x0F00) >> 4])
				V[CARRY_FLAG] = 0;
			else
				V[CARRY_FLAG] = 1;

			V[V[(opcode & 0x0F00) >> 8]] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			Log("[8XY7] Math, Vx = Vy - Vx");
			break;

		case 0x000E: // BitOp, Vx=Vy=Vy<<1
			V[CARRY_FLAG] = V[(opcode & 0x00F0) >> 4] & 0b00000001; // @TODO: Correct ?!!
			V[(opcode & 0x00F0) >> 4] <<= 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			UpdatePC();
			Log("[8XYE] BitOp, Vx=Vy=Vy<<1");
			break;

		default:
			Log("Error (decode): Bad opcode (0x8000): " + opcode);
		}
		break;

	case 0x9000: // Cond, if (Vx != Vy) skips nexts instruction
		(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) ? pc += 4 : UpdatePC();
		Log("[9XY0] Cond, skips instr. if Vx != Vy");
		break;

	case 0xA000: // Mem, sets I to address NNN
		I = opcode & 0x0FFF;
		UpdatePC();
		Log("[ANNN] MEM, I = NNN");
		break;

	case 0xB000: // Flow [0xBNNN], pc = v0 + NNN
		pc = V[0] + (opcode & 0x0FFF);
		Log("[BNNN] Flow, pc = V0 + NNN");
		break;

	case 0xC000: // Rand [0xCxNN], Vx = rand() % 255 & NN
		V[(opcode & 0x0F00) >> 8] = (rand() % 255) & (opcode & 0x00FF);
		UpdatePC();
		Log("[CXNN] Rand, Vx = rand() % 255 & NN");
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
				if ((pixel & (0x80 >> j)) != 0)
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
		Log("[DXYN] Disp, draw(Vx, Vy, N)");
	}
	break;

	case 0xE000:
		switch (opcode & 0x000F)
		{
		case 0x000E: // KeyOp, if (key() == Vx)
			(key[V[(opcode & 0x0F00) >> 8]] != 0) ? pc += 4 : UpdatePC();
			Log("[EX9E] KeyOp, skips instr. if key in Vx is pressed");
			break;

		case 0x0001: // KeyOp, if (key() != Vx)
			(key[V[(opcode & 0x0F00) >> 8]] == 0) ? pc += 4 : UpdatePC();
			Log("[EX9E] KeyOp, skips instr. if key in Vx isn't pressed");
			break;

		default:
			Log("Error(decode) : Bad opcode(0xE000) : " + opcode);
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // Timer, Vx = get_delay()
			V[(opcode & 0x0F00) >> 8] = delayTimer;
			UpdatePC();
			Log("[FX07] Timer, Vx = delayTimer");
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

			Log("[FX0A] KeyOp, Wait for a key to be pressed");
		}
		break;

		case 0x0015: // Timer, delay_timer(Vx)
			delayTimer = V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			Log("[FX15] Timer, delayTimer = Vx");
			break;

		case 0x0018: // Sound, sound_timer(Vx)
			soundTimer = V[(opcode & 0x0F00) >> 8];
			UpdatePC();
			Log("[FX18] Sound, soundTimer = Vx");
			break;

		case 0x001E: // Mem, I += Vx
			I += V[(opcode & 0x0F00) >> 8]; // @TODO: Check overflow ?!
			UpdatePC();
			Log("[FX1E] MEM, I += Vx");
			break;

		case 0x0029: // Mem, I = sprite_addr[Vx]
			I = V[(opcode & 0x0F00) >> 8] * 5;
			UpdatePC();
			Log("[FX29] MEM, Set I to te location of the sprite");
			break;

		case 0x0033: // Bcd
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
			UpdatePC();
			Log("[FX33] BCD");
			break;

		case 0x0055: // Mem, reg_dump(Vx, &I)
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
				memory[I++] = V[i];

			UpdatePC();
			Log("[FX55] MEM, reg_dump(Vx, &I)");
			break;

		case 0x0065: // Mem, reg_load(Vx, &I)
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
				V[i] = memory[I++];

			UpdatePC();
			Log("[FX65] MEM, reg_load(Vx, &I)");
			break;

		default:
			Log("Error (decode): Bad opcode (0xF000): " + opcode);
		}
		break;

	default:
		Log("Error (Decode): Unknown opcode " + opcode);
	}
}

/* Get next opcode from memory. Opcodes are stored as Big-endian. */
unsigned short Chip8::FetchOpcode()
{
	unsigned char highByte = memory[pc];
	unsigned char lowByte  = memory[pc + 1];

	return highByte << 8 | lowByte;
}

/* Decreasing timers until they reach 0. */
void Chip8::UpdateTimers()
{
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

/* Logs message to console. */
void Log(const std::string& message)
{
	std::cout << message << std::endl;
}