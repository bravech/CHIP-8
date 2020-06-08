#include <SFML/Graphics.hpp>
#include <iostream>
#include "Chip8Emu.h"




int main()
{
	Chip8Emu my_emu("Pong.ch8");
	my_emu.run();
	int a;
	std::cin >> a;
}