#include "bus.h"
#include <fstream>
#include <iostream>
#include <iomanip> 

using namespace std;

Bus::Bus(){
	cpu.connectBus(this);
}

void Bus::reset(){
	cpu.reset();
	ppu.reset();
	dmaPage = 0x0;
	dmaAddr = 0x0;
	dmaData = 0x0;
	dmaDummy = true;
	dmaData = false;
}

void Bus::writeCartridge(uint16_t address, uint8_t value){
	cartridge[address] = value;
}

void Bus::loadPpuRom(){
	for(int i = 16384; i < 24576; i++){
		ppu.ppuWrite(i - 16384, cartridge[i]);
	}
};

void Bus::loadCartridge(){
	std::ifstream myfile("donkey kong.nes");
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
		return ppu.cpuRead(address & 0x7);
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
		ppu.cpuWrite(address & 0x7, value);
	}

	if(address == 0x4014){
		dmaPage = value;
		dmaAddr = 0x00;
		dmaTransfer = true;
	}

	if(address == 0x4016 || address == 0x4017){
		// TODO: Fixed value for now
		controllerState[address & 0x1] = controller;
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
	ppu.clock();
				
	if(nesClockCount % 3 == 0){
		if(dmaTransfer){
			if(dmaDummy){
				if(nesClockCount % 2 == 1){
					dmaDummy = false;
				}
			} else {
				if(nesClockCount % 2 == 0) {
					dmaData = cpuRead(dmaPage << 8 | dmaAddr);
				} else {
					ppu.pOAM[dmaAddr] = dmaData;
					dmaAddr++;

					if(dmaAddr == 0x0){
						dmaTransfer = false;
						dmaDummy = true;
					}
				}
			}
		} else {
			cpu.clock();
		}
	}

	if(ppu.nmi){
		ppu.nmi = false;
		cpu.nmi();
	}

	nesClockCount++;
}
