#include "bus.h"
#include <fstream>
#include <iostream>
#include <iomanip> 

using namespace std;

void Bus::writeCartridge(uint16_t address, uint8_t value){
	cartridge[address] = value;
}

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

	loadPpuRom();
}

uint8_t Bus::cpuRead(uint16_t address){
	if(0 <= address && address <= 0x1FFF){
		return cpuRam[address & 0x7FF];
	}
	
	if(0x2000 <= address && address <= 0x3FFF){
		return ppu.read(0x2000 | (address & 0x7));
	}

	if(address == 0x4016 || address == 0x4017){
		uint8_t data = (controllerState[address & 0x1] & 0x80) > 0;
		controllerState[address & 0x1] <<= 1;
		return data;
	}

	if( 0x8000 <= address && address <= 0xBFFF){
		return cartridge[address - 0x8000];		
	}

	if( 0xC000 <= address && address <= 0xFFFF){
		return cartridge[address - 0xC000];		
	}

	return 0x0;
};

void Bus::cpuWrite(uint16_t address, uint8_t value){
	if( 0 <= address && address <= 0x1FFF){
		cpuRam[address & 0x7FF] = value;
	}

	if(0x2000 <= address && address <= 0x3FFF){
		ppu.write(0x2000 | (address & 0x7), value);
	}

	if(address == 0x4014){
		ppu.write(0x4014, value);
	}

	if(address == 0x4016 || address == 0x4017){
		// TODO: Fixed value for now
		controllerState[address & 0x1] = controller[address & 0x1];
	}

	if( 0x8000 <= address && address <= 0xBFFF){
		cartridge[address - 0x8000] = value;		
	}

	if( 0xC000 <= address && address <= 0xFFFF){
		cartridge[address - 0xC000] = value;		
	}
};

#include <iostream>

void Bus::clock(){
	uint16_t test = cpuRead(0xfffc);
	test |= cpuRead(0xfffd) << 8;

	test = 0xc000;
	cpu.pc = test;

	int temp = 0;
	while(temp < 16384 * 1000){
		ppu.clock();
		
		if(temp % 3 == 0){
			cpu.clock();
		}

		if(ppu.nmi){
			ppu.nmi = false;
			cpu.nmi();
		}

		temp++;
	}

	for(int i = 0; i <= 0xFD; i++){
		cout << hex << i << " " << (int)cpuRead(i) << endl;
	}
}
