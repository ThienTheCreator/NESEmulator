#pragma once

#include <cstdint>

class Bus{
	uint8_t memory[65536];
public:
	void setValue(uint16_t address, uint8_t value){ memory[address] = value; };
	uint8_t getValue(uint16_t address){ return memory[address]; };
};
