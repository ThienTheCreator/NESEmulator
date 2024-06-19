#include "bus.h"

void Bus::setValue(uint16_t address, uint8_t value){
	if(0x2000 <= address && address <= 0x2007){
		uint8_t temp = 0b00011111 & value;
		memory[0x2002] |= temp;
	}
	memory[address] = value;
};

uint8_t Bus::getValue(uint16_t address){ 
	return memory[address]; 
};
