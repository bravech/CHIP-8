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
		if (v[nibbles[1]] == v[nibbles[2]]) {
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
		case 3:
			v[nibbles[1]] = v[nibbles[1]] ^ v[nibbles[2]];
			break;
		case 4:
			v[nibbles[1]] = v[nibbles[1]] + v[nibbles[2]];
			break;
		case 5:
			v[nibbles[1]] = v[nibbles[1]] - v[nibbles[2]];
			break;
		case 6:
			v[nibbles[1]] = v[nibbles[1]] >> 1; //TODO: Double check this is proper, may use VY also
			break;
		case 7:
			v[nibbles[1]] = v[nibbles[2]] - v[nibbles[1]];
			break;
		case 0xe:
			v[nibbles[1]] = v[nibbles[1]] << 1; //TODO: Double check this is proper, may use VY also
			break;
		}
		break;
	case 9:
		if (v[nibbles[1]] != v[nibbles[2]]) {
			pc += 2;
		}
		break;
	case 0xa:
		I = (nibbles[1] << 8) + code[pc + 1];
		break;
	case 0xb:
		pc = v[0] + (nibbles[1] << 8) + code[pc + 1];
		break;
	case 0xc:
		v[nibbles[1]] = (rand() & 0xff) & code[pc + 1];
		break;
	case 0xd:
		// TODO: Implement graphics
		break;
	case 0xe:
		// TODO: Implement input / keycodes
		break;
	case 0xf:
		switch (nibbles[3]) {
		case 7:
			break;
		case 0xa:
			break;
		}
		break;


	default:
		//std::cout << "WARN: Invalid Opcode Detected! " << std::hex << (int)code[pc] << " " << (int)code[pc + 1] << std::endl;
		break;
		
	}
	pc += 2;
}

int Chip8Emu::run() {
	std::thread timerThread(&Chip8Emu::decrementTimers, this);

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

void Chip8Emu::decrementTimers() {
	using frames = std::chrono::duration < int64_t, std::ratio<1, TIMER_HZ> >;
	auto nextFrame = std::chrono::system_clock::now() + frames{ 0 };
	auto lastFrame = nextFrame - frames{ 1 };

	while (true) {
		soundTimerMutex.lock();
		if (soundTimer > 0) {
			soundTimer--;
		}
		soundTimerMutex.unlock();
		delayTimerMutex.lock();
		if (delayTimer > 0) {
			delayTimer--;
		}
		delayTimerMutex.unlock();
		std::this_thread::sleep_until(nextFrame);
		if (DEBUG) {
			std::cout << "Timer clock: " << std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now() - lastFrame).count() << std::endl;
		}
		lastFrame = nextFrame;
		nextFrame = nextFrame + frames{ 1 };
	}
}