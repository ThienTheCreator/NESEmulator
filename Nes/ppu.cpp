#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "window.h"

using namespace std;

class PPU{
	uint32_t color[64];
	uint8_t ppuctrl;
	uint8_t ppumask;
	uint8_t ppustatus;
	uint8_t oamaddr;
	uint8_t oamdata;
	uint8_t ppuscroll;
	uint8_t ppuaddr;
	uint8_t ppudata;
	uint8_t oamdma;
public:
	void setupColor(){
		color[0] = 0x55555500;
		color[1] = 0x00177300;
		color[2] = 0xffff8600;
		color[3] = 0x2e057800;
		color[4] = 0x59024d00;
		color[5] = 0x72001100;
		color[6] = 0x6e000000;
		color[7] = 0x4c080000;
		color[8] = 0x171b0000;
		color[9] = 0x002a0000;
		color[10] = 0x00310000;
		color[11] = 0x002e0800;
		color[12] = 0x00264500;
		color[13] = 0x00000000;
		color[14] = 0x00000000;
		color[15] = 0x00000000;
		color[16] = 0xffffa500;
		color[17] = 0xffffc600;
		color[18] = 0xffffe500;
		color[19] = 0xffffd900;
		color[20] = 0xffffa600;
		color[21] = 0xd2175900;
		color[22] = 0xd1210700;
		color[23] = 0xa7370000;
		color[24] = 0x63510000;
		color[25] = 0x18670000;
		color[26] = 0x00720000;
		color[27] = 0x00733100;
		color[28] = 0xffff8400;
		color[29] = 0x00000000;
		color[30] = 0x00000000;
		color[31] = 0x00000000;
		color[32] = 0xffffff00;
		color[33] = 0xffffff00;
		color[34] = 0xffffff00;
		color[35] = 0xffffff00;
		color[36] = 0xffffff00;
		color[37] = 0xffffbd00;
		color[38] = 0xff7e7500;
		color[39] = 0xff8a2b00;
		color[40] = 0xffa00000;
		color[41] = 0xffb80200;
		color[42] = 0xffc83000;
		color[43] = 0xffcd7b00;
		color[44] = 0xffffd000;
		color[45] = 0x3c3c3c00;
		color[46] = 0x00000000;
		color[47] = 0x00000000;
		color[48] = 0xffffff00;
		color[49] = 0xffffff00;
		color[50] = 0xffffff00;
		color[51] = 0xffffff00;
		color[52] = 0xffffff00;
		color[53] = 0xffffea00;
		color[54] = 0xffffc900;
		color[55] = 0xffffaa00;
		color[56] = 0xffff9600;
		color[57] = 0xffff9500;
		color[58] = 0xffffa500;
		color[59] = 0xffffc300;
		color[60] = 0xffffe600;
		color[61] = 0xffffaf00;
		color[62] = 0x00000000;
		color[63] = 0x00000000;
	}
};

PPU ppu_2C02;

int main() {
	HANDLE thread = CreateThread(NULL, 0, ep, NULL, 0, NULL);
	
	for(int i = 0; i < 255; i++){
		getidle();
		Sleep(100);
	}

	return 0;
}
