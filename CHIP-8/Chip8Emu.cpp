#include "Chip8Emu.h"
Chip8Emu::Chip8Emu(std::string rom_path) {
	for (int i = 0; i < 64 * 32; i++) screen[i] = 0;
	sf::RenderWindow temp(sf::VideoMode(WIN_SIZE, WIN_SIZE), "CHIP-8 Emulator!");
	window = &temp;

	std::ifstream romfile;
	romfile.open(rom_path, std::ios::in | std::ios::binary);

}
Chip8Emu::~Chip8Emu() {
	delete window;
}

int Chip8Emu::run() {
	while (window->isOpen()) {
		while (window->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window->close();
			} 
		}
		window->clear();
		window->display();
	}
	return 0;
}
