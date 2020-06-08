#include "Chip8Emu.h"
Chip8Emu::Chip8Emu(std::string rom_path) {
	for (int i = 0; i < 64 * 32; i++) screen[i] = 0;
	window.create(sf::VideoMode(WIN_SIZE, WIN_SIZE), "CHIP-8 Emulator!");


	std::ifstream romfile;
	romfile.open(rom_path, std::ios::in | std::ios::binary | std::ios::ate);

	std::streampos size = romfile.tellg();
	code = new uint8_t[size];
	romfile.seekg(0, std::ios::beg);
	romfile.read((char *) code, size);
	romfile.close();
}
Chip8Emu::~Chip8Emu() {
}

void Chip8Emu::execOpcode() {
	int nibbles[4] = { (code[pc] & 0xf0) >> 4, code[pc] & 0xf, (code[pc + 1] & 0xf0) >> 4, code[pc + 1] & 0xf };
	switch ((code[pc] & 0xf0) >> (4)) {
	case 1:
		pc = ((code[pc] & 0xf) << 8) + code[pc + 1];
		return;
		break;
	case 2:
		// TODO: Implement callstack
		break;
	case 3:
		if (v[nibbles[1]] == code[pc + 1]) {
			pc += 2;
		}
		break;
	case 4:
		if (v[nibbles[1]] != code[pc + 1]) {
			pc += 2;
		}
		break;
	case 5:
		if (v[nibbles[1]] != v[nibbles[2]]) {
			pc += 2;
		}
		break;
	case 6:
		v[nibbles[1]] = code[pc + 1];
		break;
	case 7:
		v[nibbles[1]] += code[pc + 1];
		break;
	case 8:
		switch (nibbles[3]) {
		case 0:
			v[nibbles[1]] = v[nibbles[2]];
			break;
		case 1:
			v[nibbles[1]] = v[nibbles[1]] | v[nibbles[2]];
			break;
		case 2:
			v[nibbles[1]] = v[nibbles[1]] & v[nibbles[2]];
			break;
		}
		break;
	default:
		std::cout << "WARN: Invalid Opcode Detected! " << std::hex << (int)code[pc] << " " << (int)code[pc + 1] << std::endl;
	}
	pc += 2;
}

int Chip8Emu::run() {
	while (window.isOpen()) {
		execOpcode();
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			} 
		}
		window.clear();
		window.display();
	}
	return 0;
}
