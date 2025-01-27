#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "ppu2C02.h"
#include "window.h"

using namespace std;

void PPU2C02::setupColor(){
	color[0]  = 0x62626200;
	color[1]  = 0x01209000;
	color[2]  = 0x240BA000;
	color[3]  = 0x47009000;
	color[4]  = 0x60006200;
	color[5]  = 0x6A002400;
	color[6]  = 0x60110000;
	color[7]  = 0x47270000;
	color[8]  = 0x243C0000;
	color[9]  = 0x014A0000;
	color[10] = 0x004F0000;
	color[11] = 0x00472400;
	color[12] = 0x00366200;
	color[13] = 0x00000000;
	color[14] = 0x00000000;
	color[15] = 0x00000000;
	color[16] = 0xABABAB00;
	color[17] = 0x1F56E100;
	color[18] = 0x4D39FF00;
	color[19] = 0x7E23EF00;
	color[20] = 0xA31BB700;
	color[21] = 0xB4226400;
	color[22] = 0xAC370E00;
	color[23] = 0x8C550000;
	color[24] = 0x5E720000;
	color[25] = 0x2D880000;
	color[26] = 0x07900000;
	color[27] = 0x00894700;
	color[28] = 0x00739D00;
	color[29] = 0x00000000;
	color[30] = 0x00000000;
	color[31] = 0x00000000;
	color[32] = 0xFFFFFF00;
	color[33] = 0x67ACFF00;
	color[34] = 0x958DFF00;
	color[35] = 0xC875FF00;
	color[36] = 0xF26AFF00;
	color[37] = 0xFF6FC500;
	color[38] = 0xFF836A00;
	color[39] = 0xE6A01F00;
	color[40] = 0xB8BF0000;
	color[41] = 0x85D80100;
	color[42] = 0x5BE33500;
	color[43] = 0x45DE8800;
	color[44] = 0x49CAE300;
	color[45] = 0x4E4E4E00;
	color[46] = 0x00000000;
	color[47] = 0x00000000;
	color[48] = 0xFFFFFF00;
	color[49] = 0xBFE0FF00;
	color[50] = 0xD1D3FF00;
	color[51] = 0xE6C9FF00;
	color[52] = 0xF7C3FF00;
	color[53] = 0xFFC4EE00;
	color[54] = 0xFFCBC900;
	color[55] = 0xF7D7A900;
	color[56] = 0xE6E39700;
	color[57] = 0xD1EE9700;
	color[58] = 0xBFF3A900;
	color[59] = 0xB5F2C900;
	color[60] = 0xB5EBEE00;
	color[61] = 0xB8B8B800;
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
	if(address == 0x2000){
		ppuGenLatch = ppuctrl.reg;
	}

	if(address == 0x2001){
		ppuGenLatch = ppumask.reg;
	}

	if(address == 0x2002){
		ppuGenLatch = ppustatus.reg;
		uint8_t temp = ppustatus.reg;
		ppustatus.V = 0;
		w = 0;
		return temp;
	}

	if(address == 0x2003){
		ppuGenLatch = oamaddr;
	}

	if(address == 0x2004){
		ppuGenLatch = oamdata;
		return oamdata;
	}

	if(address == 0x2005){
		ppuGenLatch = ppuscroll;
	}

	if(address == 0x2006){
		ppuGenLatch = ppuaddr;
	}

	if(address == 0x2007){
		ppudata = ppudataBuffer;
		ppudataBuffer = getValue(v.reg);
		
		v.reg += ppuctrl.i ? 32 : 1;
		
		ppuGenLatch = ppudata;
	}

	return ppuGenLatch;
}

// communication used by cpu
void PPU2C02::write(uint16_t address, uint8_t value){
	if(address == 0x4014){
		// TODO
		return;
	}

	if(address == 0x2000){
		ppuctrl.reg = value;
		t.nametable = value;
	} 

	if(address == 0x2001){
		ppumask.reg = value;
	}

	if(address == 0x2002){
		w = 0;
	}

	if(address == 0x2003){
		oamaddr = value;
	}

	// TODO
	if(address == 0x2004){
	}

	if(address == 0x2005){
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
	}

	if(address == 0x2006){
		if(w == 0){ // first write
			t.fineY = value >> 4;
			t.nametable = value >> 2;
			t.coarseY << 3;
			t.fineY |= 1 << 2;
			w = 1;
		} else {    // second write
			t.reg |= value;
			v = t;
			w = 0;
		}
	}

	if(address == 0x2007){
		setValue(v.reg, value);
		v.reg += ppuctrl.i ? 32 : 1;
	}

	ppuGenLatch = value;
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

	shiftRegAttributeLs = (shiftRegAttributeLs & 0xFF00) | ((nextTileAttribute & 0b01) ? 0xFF : 0x00);
	shiftRegAttributeMs = (shiftRegAttributeMs & 0xFF00) | ((nextTileAttribute & 0b10) ? 0xFF : 0x00);
}

void PPU2C02::updateShifter(){
	if(ppumask.b){
		shiftRegPatternLs <<= 1;
		shiftRegPatternMs <<= 1;

		shiftRegAttributeLs <<= 1;
		shiftRegAttributeMs <<= 1;
	}
}

uint32_t PPU2C02::getColor(uint8_t palette, uint8_t pixel){
	return color[getValue(0x3F00 + (palette << 2) + pixel) & 0x3F];
};

void PPU2C02::clock(){
	if(-1 <= scanline && scanline <= 239){
		if(scanline == -1 && cycle == 1){
			ppustatus.V = false;
		}

		if(scanline == 0 && cycle == 0)
			cycle = 1;

		if((2 <= cycle && cycle < 258) || (321 <= cycle && cycle < 338)){
			updateShifter();

			if((cycle - 1) % 8 == 0){
				loadShiftRegister();
				nextTileId = getValue(0x2000 | (v.reg & 0xFFF));
			}
			
			if((cycle - 1) % 8 == 2){
				nextTileAttribute = getValue(0x23C0 | (v.reg & 0x0C00) | ((v.reg >> 4) & 0x38) | ((v.reg >> 2) & 0x7));
			}
			
			if((cycle - 1) % 8 == 4){
				nextTileBgLs = getValue(ppuctrl.s << 12 | nextTileId << 4 | v.fineY);
			}
			
			if((cycle - 1) % 8 == 6){
				nextTileBgMs = getValue(ppuctrl.s << 12 | nextTileId << 4 | (v.fineY + 8));
			}

			if((cycle - 1) % 8 == 7){
				if((v.reg & 0x001F) == 31){
					v.reg &= ~0x001F;
					v.reg ^= 0x0400;
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
					v.reg ^= 0x0800;
				} else if(y == 31){
					y = 0;
				} else {
					y += 1;
				}
				v.reg = (v.reg & ~0x03E0) | (y << 5);
			}
		}

		if(cycle == 257){
			if(ppumask.b){
				v.reg = (v.reg & ~0x41F) | (t.reg & 0x41F);
			}
		}

		if(cycle == 338 || cycle == 340){
			nextTileId = read(0x2000 | (v.reg & 0xFFF));
		}

		if(scanline == -1 && cycle >= 280 && cycle < 305){
			
		}
	}

	if(241 <= scanline && scanline <= 260){
		if(scanline == 241 && cycle == 1){
			ppustatus.V = true;

			if(ppuctrl.v)
				nmi = true;
		}
	}

	uint8_t bg_pixel = 0;
	uint8_t bg_palette = 0;

	if(ppumask.b){
		uint16_t bit_mux = 0x8000 >> x;

		uint8_t pixelLs = (shiftRegPatternLs & bit_mux) != 0;
		uint8_t pixelMs = (shiftRegPatternMs & bit_mux) != 0;
		uint8_t bg_pixel = (pixelMs << 1) | pixelLs;

		uint8_t palLs = (shiftRegAttributeLs & bit_mux) != 0;
		uint8_t palMs = (shiftRegAttributeMs & bit_mux) != 0;
		uint8_t bg_palette = (palMs << 1) | palLs;
	}	

	if(0 <= scanline && scanline <= 240 && 0 <= cycle && cycle <= 256){
		uint32_t bgColor = getColor(bg_palette, bg_pixel);
		windowPixelColor[(scanline % 240) * 256 + ((cycle-1) % 256)] = bgColor;
		updateScreen();
	}

	cycle++;
	if(cycle >= 341){
		cycle = 0;
		scanline++;
			
		if(scanline >= 261)
			scanline = -1;
	}
}
