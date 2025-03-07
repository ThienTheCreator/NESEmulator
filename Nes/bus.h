#pragma once

#include <cstdint>
#include "cpu6502.h"
#include "ppu2C02.h"

class Bus{
	uint8_t cartridge[24576];
	uint8_t cpuRam[2048];
	uint8_t controllerState[2];
	uint8_t nesClockCount = 0;
public:
	Bus();

	void reset();

	uint16_t cycle = 0;
	CPU6502 cpu;
	PPU2C02 ppu;

	uint8_t dmaPage = 0x0;
	uint8_t dmaAddr = 0x0;
	uint8_t dmaData = 0x0;

	bool dmaDummy = true;
	bool dmaTransfer = false;

	void writeCartridge(uint16_t address, uint8_t value);
	void loadPpuRom();
	void loadCartridge();

	uint8_t cpuRead(uint16_t address);
	void cpuWrite(uint16_t address, uint8_t value);

	void clock();
};
