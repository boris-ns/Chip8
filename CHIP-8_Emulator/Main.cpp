#include <iostream>
#include "Cpu.h"

using namespace std;



int main()
{
	// @TODO: Use SFML ?
	//setupGraphics();
	//setupInput();

	Chip8 chip;
	//chip.LoadROM();
	chip.MainLoop();

	return 0;
}