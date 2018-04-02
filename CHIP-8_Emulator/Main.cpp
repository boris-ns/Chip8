#include <iostream>
#include <string>
#include "Cpu.h"

int main(int argc, char* argv[])
{
	if (argc > 2)
	{
		Log("Wrong number of command line arguments.");
		return 0;
	}

	Chip8 chip;
	std::string inputRomFile = "";

	if (argc == 2)
	{
		inputRomFile = argv[1];
	}
	else
	{
		std::cout << "Enter name of input ROM file: ";
		std::cin >> inputRomFile;
	}

	chip.LoadROM(inputRomFile);
	chip.MainLoop();

	return 0;
}