#include "Cpu.h"

int main()
{
	Chip8 chip;
	chip.LoadROM("../ROMs/BLINKY");
	chip.MainLoop();

	return 0;
}