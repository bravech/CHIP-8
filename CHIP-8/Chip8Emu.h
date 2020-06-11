#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <ios>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <stack>
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCREEN_SCALE 8
#define TIMER_HZ 60
#define CPU_HZ 500
#define DRAW_HZ 1
#define DEBUG false

class Chip8Emu
{
public:
	int main();
	int run();
	void execOpcode();
	Chip8Emu(std::string rom_path);
	void decrementTimers();
	~Chip8Emu();
private:
	uint8_t mem[0x1000]; // Memory of the CHIP-8
	uint8_t v[16]; // Registers
	uint16_t I; // Address register
	uint8_t* code;
	uint16_t pc;
	uint8_t soundTimer;
	std::mutex soundTimerMutex;
	uint8_t delayTimer;
	std::mutex delayTimerMutex;
	bool beepPlaying;
	bool running;
	std::stack<uint16_t> callstack;
	


	sf::Uint8 screen[64 * 32 * 4];
	sf::RenderWindow window;
	sf::Event event;
};

