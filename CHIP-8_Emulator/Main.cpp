#include "Cpu.h"

int main()
{
	Chip8 chip;
	chip.LoadROM("../ROMs/pong2.c8");
	chip.MainLoop();

	return 0;
}