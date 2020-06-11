#include "Chip8Emu.h"
Chip8Emu::Chip8Emu(std::string rom_path) :
	soundTimer(0), delayTimer(0) {
	memset(screen, 0, sizeof(screen));
	memset(mem, 0, sizeof(mem));
	memset(v, 0, sizeof(v));
	
	running = true;

	

	window.create(sf::VideoMode(DISPLAY_WIDTH * SCREEN_SCALE, DISPLAY_HEIGHT * SCREEN_SCALE), "CHIP-8 Emulator!");
	window.setFramerateLimit(60);


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
	using frames = std::chrono::duration < int64_t, std::ratio<1, CPU_HZ> >;
	auto nextFrame = std::chrono::system_clock::now() + frames{ 0 };
	auto lastFrame = nextFrame - frames{ 1 };

	while (running) {
		int nibbles[4] = { (code[pc] & 0xf0) >> 4, code[pc] & 0xf, (code[pc + 1] & 0xf0) >> 4, code[pc + 1] & 0xf };
		switch ((code[pc] & 0xf0) >> (4)) {
		case 0:
			switch (code[pc + 1]) {
			case 0xe0:
				memset(screen, 0, sizeof(screen));
				break;
			case 0xee:
				I = callstack.top();
				callstack.pop();
				break;
			}
		case 1:
			pc = ((code[pc] & 0xf) << 8) + code[pc + 1];
			break;
		case 2:
			callstack.push(I);
			I = (nibbles[1] << 8) + code[pc + 1];
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
			v[0xf] = 0;
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < nibbles[3]; j++) {
					for (int k = 0; k < 4; k++) {
						if (screen[4 * ((j + nibbles[2]) * 8 + i + nibbles[1]) + k] && mem[I + j * 8 + i]) v[0xf] = 1;
						screen[4 * ((j + nibbles[2]) * 8 + i + nibbles[1]) + k] ^= mem[I + j * 8 + i];
					}
				}
			}
			break;
		case 0xe:
			// TODO: Implement input / keycodes
			break;
		case 0xf:
			switch (nibbles[3]) {
			case 7:
				delayTimerMutex.lock();
				v[nibbles[1]] = delayTimer;
				delayTimerMutex.unlock();
				break;
			case 0xa:
				break;
			case 5:
				delayTimerMutex.lock();
				delayTimer = v[nibbles[1]];
				delayTimerMutex.unlock();
				break;
			case 8:
				soundTimerMutex.lock();
				soundTimer = v[nibbles[1]];
				soundTimerMutex.unlock();
				break;
			case 0xe:
				v[0xf] = 0;
				if (I + v[nibbles[1]] > 0xfff) {
					v[0xf] = 1;
				}
				I += v[nibbles[1]];
				break;
			}
			break;


		default:
			std::cout << "WARN: Invalid Opcode Detected! " << std::hex << (int)code[pc] << " " << (int)code[pc + 1] << std::endl;
			break;
		}
		pc += 2;
		std::this_thread::sleep_until(nextFrame);
		if (DEBUG) {
			std::cout << "CPU clock: " << std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now() - lastFrame).count() << std::endl;
		}
		lastFrame = nextFrame;
		nextFrame = nextFrame + frames{ 1 };
	}
}

int Chip8Emu::run() {
	std::thread timerThread(&Chip8Emu::decrementTimers, this);
	std::thread cpuThread(&Chip8Emu::execOpcode, this);

	sf::Image screenImage;
	sf::Texture screenTexture;
	sf::Sprite screenSprite;
	
	

	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				running = false;
			} 
		}
		window.clear();
		screenImage.create(DISPLAY_WIDTH, DISPLAY_HEIGHT, screen);
		screenTexture.loadFromImage(screenImage);
		screenSprite.setTexture(screenTexture, true);
		screenSprite.setScale(SCREEN_SCALE, SCREEN_SCALE);

		window.draw(screenSprite);
		window.display();
	}
	timerThread.join();
	cpuThread.join();
	return 0;
}

void Chip8Emu::decrementTimers() {
	using frames = std::chrono::duration < int64_t, std::ratio<1, TIMER_HZ> >;
	auto nextFrame = std::chrono::system_clock::now() + frames{ 0 };
	auto lastFrame = nextFrame - frames{ 1 };

	while (running) {
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