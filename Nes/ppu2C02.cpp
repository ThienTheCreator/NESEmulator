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

uint8_t PPU2C02::getValue(uint16_t address){

	if(0x000 <= address && address <= 0xFFF){
		return patternTable[0][address];

	}else if(0x1000 <= address && address <= 0x1FFF){
		return patternTable[1][address & 0xFFF];

	}else if(0x2000 <= address && address <= 0x3EFF){
		address &= 0x0FFF;
		
		if (0x000 <= address && address <= 0x3FF){
			return nameTable[0][address & 0x3FF];

		}else if(0x400 <= address && address <= 0x7FF){
			return nameTable[0][address & 0x3FF];

		}else if(0x800 <= address && address <= 0xBFF){
			return nameTable[1][address & 0x3FF];

		}else if(0xC00 <= address && address <= 0xFFF){
			return nameTable[1][address & 0x3FF];
		}
	} else if(0x3F00 <= address && address <= 0x3FFF){
		address &= 0x1F;
		if(address == 0x10) 
			address = 0x0;

		if(address == 0x14)
			address = 0x4;

		if(address == 0x18)
			address = 0x8;

		if(address == 0x1C)
			address = 0xC;

		return paletteTable[address] & (ppumask.Greyscale ? 0x30 : 0x3F);
	}

	return 0;
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
	} else if(0x3F00 <= address && address <= 0x3FFF){
		address &= 0x1F;
		if(address == 0x10) 
			address = 0x0;

		if(address == 0x14)
			address = 0x4;

		if(address == 0x18)
			address = 0x8;

		if(address == 0x1C)
			address = 0xC;

		paletteTable[address] = value;
	}

}

// communication used by cpu
uint8_t PPU2C02::read(uint16_t address){
	if(address == 0x2002){
		ppuGenLatch = (ppustatus.reg & 0xE0) | (ppuGenLatch & 0x1F);
		ppustatus.V = 0;
		w = 0;
	}

	if(address == 0x2004){
		ppuGenLatch = pOAM[oamAddr];
	}

	if(address == 0x2007){
		ppuGenLatch = ppuDataBuffer;
		ppuDataBuffer = getValue(v.reg);

		if(v.reg >= 0x3F00) 
			ppuGenLatch = ppuDataBuffer;
		
		v.reg += (ppuctrl.i ? 32 : 1);
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
		t.nametable = ppuctrl.nn;
	} 

	if(address == 0x2001){
		ppumask.reg = value;
	}

	if(address == 0x2003){
		oamAddr = value;
	}

	if(address == 0x2004){
		pOAM[oamAddr] = value;
	}

	if(address == 0x2005){
		if(w == 0){ // first write
			x = value & 0x7;
			t.coarseX = ppuScroll >> 3;
			w = 1;
		} else {    // second write
			t.fineY = value & 0x7;
			t.coarseY = ppuScroll >> 3;
			w = 0;
		}
	}

	if(address == 0x2006){
		if(w == 0){ // first write
			t.reg = (uint16_t)((value & 0x3F) << 8) | (t.reg & 0x00FF);
			w = 1;
		} else {    // second write
			t.reg = (t.reg & 0xFF00) | value;
			v = t;
			w = 0;
		}
	}

	if(address == 0x2007){
		setValue(v.reg, value);
		v.reg += (ppuctrl.i ? 32 : 1);
	}

	ppuGenLatch = value;
}

void PPU2C02::reset(){
	cycle = 0;

	oamAddr = 0;
	ppuScroll = 0;
	ppuAddr = 0;
	ppuData = 0;
}

void PPU2C02::loadBgShifter(){
	bgShifterPatternLs = (bgShifterPatternLs & 0xFF00) | nextTileBgLs;
	bgShifterPatternMs = (bgShifterPatternMs & 0xFF00) | nextTileBgMs;

	bgShifterAttributeLs = (bgShifterAttributeLs & 0xFF00) | ((nextTileAttribute & 0b01) ? 0xFF : 0x00);
	bgShifterAttributeMs = (bgShifterAttributeMs & 0xFF00) | ((nextTileAttribute & 0b10) ? 0xFF : 0x00);
}

void PPU2C02::updateShifter(){
	if(ppumask.bg){
		bgShifterPatternLs <<= 1;
		bgShifterPatternMs <<= 1;

		bgShifterAttributeLs <<= 1;
		bgShifterAttributeMs <<= 1;
	}

	if(ppumask.s && 1 <= cycle && cycle < 258){
		for(int i = 0; i < spriteCount; ++i){
			if(spriteScanline[i].x > 0){
				spriteScanline[i].x--;
			} else {
				spriteShifterPatternLs[i] <<= 1;
				spriteShifterPatternMs[i] <<= 1;
			}
		}
	}
}

uint32_t PPU2C02::getColor(uint8_t palette, uint8_t pixel){
	return color[getValue(0x3F00 + (palette << 2) + pixel) & 0x3F];
};

void PPU2C02::clock(){
	if(-1 <= scanline && scanline <= 239){
		if(scanline == -1 && cycle == 1){
			ppustatus.V = 0;
			ppustatus.S = 0;
			ppustatus.O = 0;

			for(int i = 0; i < 8; ++i){
				spriteShifterPatternLs[i] = 0;
				spriteShifterPatternMs[i] = 0;
			}	
		}

		if(scanline == 0 && cycle == 0)
			cycle = 1;

		if((2 <= cycle && cycle < 258) || (321 <= cycle && cycle < 338)){
			updateShifter();

			if((cycle - 1) % 8 == 0){
				loadBgShifter();
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
			loadBgShifter();
			if(ppumask.bg || ppumask.s){
				v.reg = (v.reg & ~0x41F) | (t.reg & 0x41F);
			}
		}

		if(cycle == 338 || cycle == 340){
			nextTileId = getValue(0x2000 | (v.reg & 0xFFF));
		}

		if(scanline == -1 && cycle >= 280 && cycle < 305){
			if(ppumask.bg || ppumask.s){
				v.fineY = t.fineY;
				v.nametable = v.nametable & (t.nametable | 1);
				v.coarseY = t.coarseY;
			}
		}

		if(cycle == 257 && scanline >= 0){
			std::memset(spriteScanline, 0xFF, 8 * sizeof(spriteObject));
			spriteCount = 0;

			for(int i = 0; i < 8; ++i){
				spriteShifterPatternLs[i] = 0;
				spriteShifterPatternMs[i] = 0;
			}

			uint8_t nOAMEntry = 0;

			bSpriteZeroHitPossible = false;

			while(nOAMEntry < 64 && spriteCount < 9){
				int16_t diff = ((int16_t) scanline - (int16_t)oam[nOAMEntry].y);

				if(diff >= 0 && diff < (ppuctrl.h ? 16 : 8)){
					if(spriteCount < 0){
						if(nOAMEntry == 0){
							bSpriteZeroHitPossible = true;
						}

						memcpy(&spriteScanline[spriteCount], &oam[nOAMEntry], sizeof(spriteObject));
						spriteCount++;
					}
				}

				nOAMEntry++;
			}

			ppustatus.O = (spriteCount > 8);
		}
		
		if(cycle == 340){
			for(int i = 0; i < spriteCount; ++i){
				uint8_t spritePatternBitsLs, spritePatternBitsMs;
				uint16_t spritePatternAddrLs, spritePatternAddrMs;

				if(!ppuctrl.h){
					if(!(spriteScanline[i].attribute & 0x80)){
						spritePatternAddrLs = (ppuctrl.s << 12) 
							| (spriteScanline[i].index << 4) 
							| (scanline - spriteScanline[i].y);
					} else {
						spritePatternAddrLs = (ppuctrl.s << 12)
							| (spriteScanline[i].index << 4)
							| (7 - (scanline - spriteScanline[i].y));
					}
				} else {
					if(!(spriteScanline[i].attribute & 0x80)){
						if(scanline - spriteScanline[i].y < 8){
							spritePatternAddrLs = ((spriteScanline[i].index & 0x1) << 12)
								| ((spriteScanline[i].index & 0xFE) << 4)
								| ((scanline - spriteScanline[i].y) & 0x7);
						} else {
							spritePatternAddrLs = ((spriteScanline[i].index & 0x1) << 12)
								| (((spriteScanline[i].index & 0xFE) + 1) << 4)
								| ((scanline - spriteScanline[i].y) & 0x7);
						}
					} else {
						if(scanline - spriteScanline[i].y < 8){
							spritePatternAddrLs = ((spriteScanline[i].index & 0x1) << 12)
								| (((spriteScanline[i].index & 0xFE) + 1) << 4)
								| (7 - (scanline - spriteScanline[i].y) & 0x7);
						} else {
							spritePatternAddrLs = ((spriteScanline[i].index & 0x1) << 12)
								| ((spriteScanline[i].index & 0xFE) << 4)
								| (7 - (scanline - spriteScanline[i].y) & 0x7);
						}
					}
				}

				spritePatternAddrMs = spritePatternAddrLs + 8;

				spritePatternBitsLs = getValue(spritePatternAddrLs);
				spritePatternBitsMs = getValue(spritePatternAddrMs);

				if(spriteScanline[i].attribute & 0x40){
					auto flipbyte = [](uint8_t b){
						b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
						b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
						b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
						return b;
					};

					spritePatternBitsLs = flipbyte(spritePatternBitsLs);
					spritePatternBitsMs = flipbyte(spritePatternBitsMs);
				}

				spriteShifterPatternLs[i] = spritePatternBitsLs;
				spriteShifterPatternMs[i] = spritePatternBitsMs;
			}
		}
	}

	if(241 <= scanline && scanline <= 260){
		if(scanline == 241 && cycle == 1){
			ppustatus.V = 1;

			if(ppuctrl.v)
				nmi = true;
		}
	}

	uint8_t bgPixel = 0;
	uint8_t bgPalette = 0;

	if(ppumask.bg){
		uint16_t bit_mux = 0x8000 >> x;

		uint8_t pixelLs = (bgShifterPatternLs & bit_mux) != 0;
		uint8_t pixelMs = (bgShifterPatternMs & bit_mux) != 0;
		uint8_t bgPixel = (pixelMs << 1) | pixelLs;

		uint8_t palLs = (bgShifterAttributeLs & bit_mux) != 0;
		uint8_t palMs = (bgShifterAttributeMs & bit_mux) != 0;
		uint8_t bgPalette = (palMs << 1) | palLs;
	}

	uint8_t fgPixel = 0;
	uint8_t fgPalette = 0;
	uint8_t fgPriority = 0;

	if(ppumask.s){
		for(int i = 0; i < spriteCount; ++i){
			if(spriteScanline[i].x == 0){
				uint8_t fgPixelLs = (spriteShifterPatternLs[i] & 0x80) > 0;
				uint8_t fgPixelMs = (spriteShifterPatternMs[i] & 0x80) > 0;
				fgPixel = (fgPixelMs << 1) | fgPixelLs;

				fgPalette = (spriteScanline[i].attribute & 0x03) + 0x04;
				fgPriority = (spriteScanline[i].attribute & 0x20) == 0;
			
				if(fgPixel != 0){
					if(i == 0){
						bSpriteZeroBeingRendered = true;
					}

					break;
				}
			}
		}
	}

	uint8_t pixel = 0;
	uint8_t palette = 0;

	if(bgPixel == 0 && fgPixel == 0){
		pixel = 0;
		palette = 0;
	} else if(bgPixel == 0 && fgPixel > 0){
		pixel = fgPixel;
		palette = fgPalette;
	} else if(bgPixel > 0 && fgPixel == 0){
		pixel = bgPixel;
		palette = bgPalette;
	} else if(bgPixel > 0 && fgPixel > 0){
		if(fgPriority){
			pixel = fgPixel;
			palette = fgPalette;
		} else {
			pixel = bgPixel;
			palette = bgPalette;
		}

		if(bSpriteZeroHitPossible && bSpriteZeroBeingRendered){
			if(ppumask.bg & ppumask.s){
				if(~(ppumask.bg | ppumask.s)){
					if(9 <= cycle && cycle < 258){
						ppustatus.S = 1;
					}
				} else {
					if(1 <= cycle && cycle < 258){
						ppustatus.S = 1;
					}
				}
			}
		}
	}

	if(0 <= scanline && scanline <= 240 && 0 <= cycle && cycle <= 256){
		uint32_t bgColor = getColor(palette, pixel);
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
