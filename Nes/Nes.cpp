#include <iostream>
#include <cstdint>

using namespace std;

class MOS_6502 {
	// CPU registers
	uint8_t a;   // arithmetic logic unit (ALU)
	uint8_t x;   // index x
	uint8_t y;   // index y
	uint16_t pc; // program counter
	uint8_t s;   // stack pointer
	
	/* 
	7  bit  0
	---- ----
	NV1B DIZC
	|||| ||||
	|||| |||+- Carry
	|||| ||+-- Zero
	|||| |+--- Interrupt Disable
	|||| +---- Decimal
	|||+------ (No CPU effect; see: the B flag)
	||+------- (No CPU effect; always pushed as 1)
	|+-------- Overflow
	V+--------- Negative

	Diagram from https://www.nesdev.org/wiki/Status_flags
	*/
	uint8_t p;   // status register

	void instructions(opcode){
		// BRK - Force Interrupt
		if(opcode == 0x00){
			mem[s] = pc >> 8;
			s++;
			mem[s] = (uint8_t)pc;
			s++;
			mem[s] = p;
			s++;
			s = s | 0b00010000;
		}
	}
};

class Bus{
	uint8_t memory[65536];
};

MOS_6502 cpu;
Bus bus;

int main() {
	return 0;
}
