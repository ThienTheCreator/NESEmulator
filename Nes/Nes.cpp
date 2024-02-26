#include <iostream>
#include <cstdint>

using namespace std;

class Nes {
	// CPU registers
	uint8_t a;   // arithmetic logic unit (ALU)
	uint8_t x;   // index x
	uint8_t y;   // index y
	uint16_t pc; // program counter
	uint8_t s;   // stack pointer
	uint8_t p;   // status register

	uint8_t cRAM[65536]; // CPU memory
};

int main() {
	return 0;
}
