#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <ios>
#define WIN_SIZE 400

class Chip8Emu
{
public:
	int main();
	int run();
	Chip8Emu(std::string rom_path);
	~Chip8Emu();
private:
	uint8_t mem[0x1000]; // Memory of the CHIP-8
	uint8_t v[16]; // Registers
	uint16_t I; // Address register
	uint8_t* code;

	sf::Uint8 screen[64 * 32];
	sf::RenderWindow* window;
	sf::Event event;
};

