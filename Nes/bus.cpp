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

	// loadPpuRom();
}

void Bus::cpuWrite(uint16_t address, uint8_t value){
	if(address == 0x2006){
		ppu.ppuaddr = value;
	}
	if(address == 0x2007){
		ppu.ppudata = value;
	}
	if( 0 <= address && address <= 0x1FFF){
		cpuRam[address] = value;
	}
};

uint8_t Bus::cpuRead(uint16_t address){
	if(0 <= address && address <= 0x1FFF){
		return cpuRam[address];
	}
	if(address == 0x2002){
		uint8_t temp = ppu.status.reg;
		ppu.status.V = 0;
		return temp;
	}
	if(address == 0x2007){
		return ppu.ppudata;
	}

	if( 0x8000 <= address && address <= 0xBFFF){
		return cartridge[address - 0x8000];		
	}

	if( 0xC000 <= address && address <= 0xFFFF){
		return cartridge[address - 0xC000];		
	}

	return 0xFF;
};

uint8_t Bus::ppuRead(uint16_t address){
	return ppu.read(address);
}

void Bus::ppuWrite(uint16_t address, uint8_t value){
	ppu.write(address, value);
}

void Bus::clock(){
	uint16_t test = cpuRead(0xfffc);
	test |= cpuRead(0xfffd) << 8;

	cpu.pc = test;

	int temp = 0;
	int instructionCount = 0;
	while(temp < 16384 * 3 && instructionCount < 128){
		if(temp % 3 == 0){
			if(cpu.waitCycle <= 0){
				if(1){ // debug
					cout << uppercase << hex;
					cout << "PC:" << setw(4) << cpu.pc;
					cout << " A:" << setfill('0') << setw(2) << (int)cpu.a;
					cout <<	" X:" << setfill('0') << setw(2) << (int)cpu.x;
					cout << " Y:" << setfill('0') << setw(2) << (int)cpu.y; 
					cout << " S:" << setfill('0') << setw(2) << (int)cpu.s;
					cout << " P:";
					cout << (0b10000000 & cpu.p ? "N" : "n");
					cout << (0b01000000 & cpu.p ? "V" : "v");
					cout << "u";
					cout << (0b00010000 & cpu.p ? "B" : "b");
					cout << (0b00001000 & cpu.p ? "D" : "d");
					cout << (0b00000100 & cpu.p ? "I" : "i");
					cout << (0b00000010 & cpu.p ? "Z" : "z");
					cout << (0b00000001 & cpu.p ? "C" : "c");
					cout << endl;
				}
				cpu.executeInstruction(cpuRead(cpu.pc));
				instructionCount++;
			} else {
				cpu.waitCycle--;
			}
		}
		if(1){
		 	ppu.executePPU();
		}
		temp++;
	}
}
