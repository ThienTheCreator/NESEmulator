#pragma once

#include <cstdint>
#include "cpu6502.h"
#include "ppu2C02.h"

class Bus{
	uint8_t cartridge[24576];
	uint8_t cpuRam[2048];
public:
	uint16_t cycle = 0;
	CPU6502 cpu;
	PPU2C02 ppu;

	void writeCartridge(uint16_t address, uint8_t value);
	void loadPpuRom();
	void loadCartridge();

	uint8_t ppuRead(uint16_t);
	void ppuWrite(uint16_t address, uint8_t value);

	uint8_t cpuRead(uint16_t address);
	void cpuWrite(uint16_t address, uint8_t value);

	void clock();
};
