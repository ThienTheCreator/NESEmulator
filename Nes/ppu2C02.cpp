#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "ppu2C02.h"
#include "window.h"

using namespace std;

void PPU2C02::setupColor(){
	color[0]  = 0x55555500;
	color[1]  = 0x00177300;
	color[2]  = 0xffff8600;
	color[3]  = 0x2e057800;
	color[4]  = 0x59024d00;
	color[5]  = 0x72001100;
	color[6]  = 0x6e000000;
	color[7]  = 0x4c080000;
	color[8]  = 0x171b0000;
	color[9]  = 0x002a0000;
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

PPU2C02::PPU2C02(){
	setupColor();
}

void PPU2C02::setValue(uint16_t address, uint8_t value){
	
	if(0x000 <= address && address <= 0xFFF){
		patternTable[0][address] = value;

	}else if(0x1000 <= address && address <= 0x1FFF){
		patternTable[1][address & 0xFFF] = value;

	}else if(0x2000 <= address && address <= 0x23FF){ // horizontal mirroring
		nameTable[0][address & 0x3FF] = value;

	}else if(0x2400 <= address && address <= 0x27FF){
		nameTable[0][address & 0x3FF] = value;

	}else if(0x2800 <= address && address <= 0x2BFF){
		nameTable[1][address & 0x3FF] = value;

	}else if(0x2C00 <= address && address <= 0x2FFF){
		nameTable[1][address & 0x3FF] = value;
	}
}

uint8_t PPU2C02::getValue(uint16_t address){

	if(0x000 <= address && address <= 0xFFF){
		return patternTable[0][address];

	}else if(0x1000 <= address && address <= 0x1FFF){
		return patternTable[1][address & 0xFFF];

	}else if(0x2000 <= address && address <= 0x23FF){
		return nameTable[0][address & 0x3FF];

	}else if(0x2400 <= address && address <= 0x27FF){
		return nameTable[0][address & 0x3FF];

	}else if(0x2800 <= address && address <= 0x2BFF){
		return nameTable[1][address & 0x3FF];

	}else if(0x2C00 <= address && address <= 0x2FFF){
		return nameTable[1][address & 0x3FF];
	}

	return 1;
}

// communication used by cpu
uint8_t PPU2C02::read(uint16_t address){
	if(address == 0x2002){
		w = 0;
	}
	return 0;
}

// communication used by cpu
void PPU2C02::write(uint16_t address, uint8_t value){
	if(address == 0x2000){
		control.reg = value;
		t.nametable = value;
	} else if(address == 0x2005){
		ppuscroll = value;
		if(w == 0){ // first write
			t.coarseX = ppuscroll >> 3;
			x = ppuscroll;
			w = 1;
		} else {    // second write
			t.fineY = ppuscroll;
			t.coarseY = ppuscroll >> 3;
			w = 0;
		}
	} else if(address == 0x2006){
		ppuaddr = value;
		if(w == 0){ // first write
			t.fineY = ppuaddr >> 4;
			t.nametable = ppuaddr >> 2;
			t.coarseY << 3;
			t.fineY |= 1 << 2;
			w = 1;
		} else {    // second write
			t.reg |= ppuaddr;
			v = t;
			w = 0;
		}
	}
}

void PPU2C02::reset(){
	cycle = 0;

	oamaddr = 0;
	ppuscroll = 0;
	ppuaddr = 0;
	ppudata = 0;
}

void PPU2C02::loadShiftRegister(){
	shiftRegPatternLs = (shiftRegPatternLs & 0xFF00) | nextTileBgLs;
	shiftRegPatternMs = (shiftRegPatternMs & 0xFF00) | nextTileBgMs;
}

uint32_t PPU2C02::getColor(uint8_t palette, uint8_t pixel){
	return color[getValue(0x3F00 + (palette << 2) + pixel) & 0x3F];
};

void PPU2C02::executePPU(){
	if(0 <= scanline && scanline <= 239){
		if((1 <= cycle && cycle <= 256) || (321 <= cycle && cycle <= 336)){
			if((cycle - 1) % 8 == 0){
				loadShiftRegister();
				nextTileId = getValue(0x2000 | (v.reg & 0xFFF));
			}
			if((cycle - 1) % 8 == 2){
				nextTileAttribute = getValue(0x23C0 | (v.reg & 0x0C00) | ((v.reg >> 4) & 0x38) | ((v.reg >> 2) & 0x7));
				shiftRegAttributeLs = nextTileAttribute & 0b01 ? 1 : 0;
				shiftRegAttributeMs = nextTileAttribute & 0b10 ? 2 : 0;
			}
			if((cycle - 1) % 8 == 4){
				nextTileBgLs = getValue(control.s << 12 | nextTileId << 4 | v.fineY);
			}
			if((cycle - 1) % 8 == 6){
				nextTileBgMs = getValue(control.s << 12 | nextTileId << 4 | (v.fineY + 8));
			}
			if((cycle - 1) % 8 == 7){
				if((v.reg & 0x001F) == 31){
					v.reg &= ~0x001F;
					v.reg ^= 0x04000;
				} else {
					v.reg += 1;
				}
			}
		}
	
		if(cycle == 256){
			if((v.reg & 0x7000) != 0x7000){
				v.reg += 0x1000;
			} else {
				v.reg &= ~0x7000;
				uint16_t y = (v.reg & 0x03E0) >> 5;
				if (y == 29){
					y = 0;
					v.reg ^= 0x8000;
				} else if(y == 31){
					y = 0;
				} else {
					y += 1;
				}
				v.reg = (v.reg & ~0x03E0) | (y << 5);
			}
		}

		if(cycle == 257){
			v.reg = (v.reg & ~0x41F) | (t.reg & 0x41F);
		}

		uint8_t bg_pixel = 0;
		uint8_t bg_palette = 0;
		if(mask.b){
			uint16_t bit_mux = 0x8000 >> x;

			uint8_t pixelLs = (shiftRegPatternLs & bit_mux) != 0;
			uint8_t pixelMs = (shiftRegPatternMs & bit_mux) != 0;
			uint8_t bg_pixel = (pixelMs << 1) | pixelLs;

			uint8_t bg_palette = (shiftRegPatternMs & 0b10) | (shiftRegPatternLs & 0b01);
		}	

		uint8_t bgColor = getColor(bg_palette, bg_pixel);
		windowPixelColor[cycle % 61440] = bgColor;
		for(int i = 0; i < 61440; i++){
			windowPixelColor[i] = 0xFFFFFF00;
		}
		updateScreen(windowPixelColor);

		cycle++;
		if(cycle >= 341){
			cycle = 0;
			scanline++;
			
			if(scanline >= 261){
				scanline = -1;
			}
		}
	}
}

/*
PPU ppu_2C02;

int main() {
	HANDLE thread = CreateThread(NULL, 0, ep, NULL, 0, NULL);
	
	for(int i = 0; i < 255; i++){
		getidle();
		Sleep(100);
	}

	return 0;
}
*/
