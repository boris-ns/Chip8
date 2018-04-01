# Chip8 Emulator (interpreter)
This is simple [Chip8](https://en.wikipedia.org/wiki/CHIP-8) emulator written in C++. [SFML library](https://www.sfml-dev.org/) is used for graphics. 
SFML is included in `Libs` folder, and games/chip8 programs are in `ROMs` folder.
Project structure is basic Microsoft Visual Studio project. If you want to run this program just open `CHIP-8_Emulator.sln` with Visual Studio and compile it. For development I used Microsoft Visual C++ compiler.
Special thanks to the author of [this article](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/). If you want to build Chip8 emulator I suggest you start from that article. All of the opcodes and it's descriptions are on [Chip8 Wikipedia page](https://en.wikipedia.org/wiki/CHIP-8).
All ROMs are downloaded from here: `https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html`.

For now you need to specify yourself location of ROM you want to run in main function in `Main.cpp`.

### Keyboard layout
```
  Chip8                  Keyboard
|1|2|3|C|                |1|2|3|4|
|4|5|6|D|                |Q|W|E|R|     
|7|8|9|E|                |A|S|D|F|
|A|0|B|F|                |Z|X|C|V|
```
