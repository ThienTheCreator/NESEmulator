#include "bus.h"
#include <fstream>

void Bus::writeCartridge(uint16_t address, uint8_t value){
	cartridge[address] = value;
}

void Bus::loadCpuRom(){
	for(int i = 0; i < 16384; i++){
		cpuWrite(0x8000 + i, cartridge[i]);
	}
};

void Bus::loadPpuRom(){
	for(int i = 16384; i < 24576; i++){
		ppu.setValue(i - 16384, cartridge[i]);
	}
};

void Bus::loadCartridge(){
	std::ifstream myfile("nestest.nes");
	char c;
	for(int i = 0; i < 16; i++){
		myfile.get(c);
	}

	for(int i = 0; i < 24576; i++){
		myfile.get(c);
		writeCartridge(i, c);
	}

	myfile.close();

	loadCpuRom();
	loadPpuRom();
}

void Bus::cpuWrite(uint16_t address, uint8_t value){
	if(address == 0x2006){
		ppu.ppuaddr = value;
	}
	if(address == 0x2007){
		ppu.ppudata = value;
	}
	if( 0 <= address && address <= 0x1FFF){
		cpuRam[address % 0x2000] = value;
	}
};

uint8_t Bus::cpuRead(uint16_t address){
	if(address == 0x2002){
		uint8_t temp = ppu.status.reg;
		ppu.status.V = 0;
		return temp;
	}
	if(address == 0x2007){
		return ppu.ppudata;
	}

	if( 0 <= address && address <= 0x1FFF){
		return cpuRam[address % 0x2000];
	}

	return 0xFF;
};

uint8_t Bus::ppuRead(uint16_t address){
	return ppu.read(address);
}

void Bus::ppuWrite(uint16_t address, uint8_t value){
	ppu.write(address, value);
}


