#include "Cpu.h"

int main()
{
	// @TODO: Use SFML ?
	//setupGraphics();
	//setupInput();

	Chip8 chip;
	//chip.LoadROM("pong.c8");
	chip.MainLoop();

	return 0;
}