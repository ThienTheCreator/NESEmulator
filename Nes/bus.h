#pragma once

#include <cstdint>
#include "cpu6502.h"
#include "ppu.h"

class Bus{
	uint8_t memory[65536];
public:
	uint16_t cycle = 0;
	CPU6502 cpu;
	PPU ppu2C02;

	void setValue(uint16_t address, uint8_t value);
	uint8_t getValue(uint16_t address);
};
