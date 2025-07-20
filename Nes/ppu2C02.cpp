#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "ppu2C02.h"
#include "window.h"

using namespace std;

PPU2C02::PPU2C02(){
	color[0]  = 0x626262;
	color[1]  = 0x001FB2;
	color[2]  = 0x2404C8;
	color[3]  = 0x5200B2;
	color[4]  = 0x730076;
	color[5]  = 0x800024;
	color[6]  = 0x730B00;
	color[7]  = 0x522800;
	color[8]  = 0x244400;
	color[9]  = 0x005700;
	color[10] = 0x005C00;
	color[11] = 0x005324;
	color[12] = 0x003C76;
	color[13] = 0x000000;
	color[14] = 0x000000;
	color[15] = 0x000000;
	color[16] = 0xABABAB;
	color[17] = 0x0D57FF;
	color[18] = 0x4B30FF;
	color[19] = 0x8A13FF;
	color[20] = 0xBC08D6;
	color[21] = 0xD21269;
	color[22] = 0xC72E00;
	color[23] = 0x9D5400;
	color[24] = 0x607B00;
	color[25] = 0x209800;
	color[26] = 0x00A300;
	color[27] = 0x009942;
	color[28] = 0x007DB4;
	color[29] = 0x000000;
	color[30] = 0x000000;
	color[31] = 0x000000;
	color[32] = 0xFFFFFF;
	color[33] = 0x53AEFF;
	color[34] = 0x9085FF;
	color[35] = 0xD365FF;
	color[36] = 0xFF57FF;
	color[37] = 0xFF5DCF;
	color[38] = 0xFF7757;
	color[39] = 0xFA9E00;
	color[40] = 0xBDC700;
	color[41] = 0x7AE700;
	color[42] = 0x43F611;
	color[43] = 0x26EF7E;
	color[44] = 0x2CD5F6;
	color[45] = 0x4E4E4E;
	color[46] = 0x000000;
	color[47] = 0x000000;
	color[48] = 0xFFFFFF;
	color[49] = 0xB6E1FF;
	color[50] = 0xCED1FF;
	color[51] = 0xE9C3FF;
	color[52] = 0xFFBCFF;
	color[53] = 0xFFBDF4;
	color[54] = 0xFFC6C3;
	color[55] = 0xFFD59A;
	color[56] = 0xE9E681;
	color[57] = 0xCEF481;
	color[58] = 0xB6FB9A;
	color[59] = 0xA9FAC3;
	color[60] = 0xA9F0F4;
	color[61] = 0xB8B8B8;
	color[62] = 0x000000;
	color[63] = 0x000000;
}

void PPU2C02::reset(){
	x = 0;
	w = 0;
	ppuDataBuffer = 0;
	scanline = 0;
	cycle = 0;
	bgNextTileId = 0;
	bgNextTileAttribute = 0;
	bgNextTileLs = 0;
	bgNextTileMs = 0;
	bgShifterPatternLs = 0;
	bgShifterPatternMs = 0;
	bgShifterAttributeLs = 0;
	bgShifterAttributeMs = 0;
	ppustatus.reg = 0;
	ppumask.reg = 0;
	ppuctrl.reg = 0;
	v.reg = 0;
	t.reg = 0;
}

uint8_t PPU2C02::ppuRead(uint16_t address){
	address &= 0x3FFF;

	if(0x000 <= address && address <= 0xFFF){
		return patternTable[0][address];

	}else if(0x1000 <= address && address <= 0x1FFF){
		return patternTable[1][address & 0xFFF];

	}else if(0x2000 <= address && address <= 0x3EFF){
		address &= 0x0FFF;
	
		// Horizontal Mirror
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

		return paletteTable[address] & (ppumask.greyscale ? 0x30 : 0x3F);
	}

	return 0;
}

void PPU2C02::ppuWrite(uint16_t address, uint8_t value){
	address &= 0x3FFF;

	if(0x000 <= address && address <= 0xFFF){
		patternTable[0][address] = value;

	}else if(0x1000 <= address && address <= 0x1FFF){
		patternTable[1][address & 0xFFF] = value;

	}else if(0x2000 <= address && address <= 0x3EFF){
		address &= 0xFFF;

		// horizontal mirroring
		if(0x000 <= address && address <= 0x3FF){
			nameTable[0][address & 0x3FF] = value;

		}else if(0x400 <= address && address <= 0x7FF){
			nameTable[0][address & 0x3FF] = value;

		}else if(0x800 <= address && address <= 0xBFF){
			nameTable[1][address & 0x3FF] = value;

		}else if(0xC00 <= address && address <= 0xFFF){
			nameTable[1][address & 0x3FF] = value;
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

		paletteTable[address] = value;
	}

}

// communication used by cpu
uint8_t PPU2C02::cpuRead(uint16_t address){
	ppuGenLatch = 0;
	if(address == 0x2){
		ppuGenLatch = (ppustatus.reg & 0xE0) | (ppuDataBuffer & 0x1F);
		ppustatus.vBlank = 0;
		w = 0;
	}

	if(address == 0x4){
		ppuGenLatch = pOAM[oamAddr];
	}

	if(address == 0x7){
		ppuGenLatch = ppuDataBuffer;
		ppuDataBuffer = ppuRead(v.reg);

		if(v.reg >= 0x3F00) 
			ppuGenLatch = ppuDataBuffer;
		
		v.reg += (ppuctrl.incrementMode ? 32 : 1);
	}

	return ppuGenLatch;
}

// communication used by cpu
void PPU2C02::cpuWrite(uint16_t address, uint8_t value){
	if(address == 0x0){
		ppuctrl.reg = value;
		t.nametableX = ppuctrl.nametableX;
		t.nametableY = ppuctrl.nametableY;
	} 

	if(address == 0x1){
		ppumask.reg = value;
	}

	if(address == 0x3){
		oamAddr = value;
	}

	if(address == 0x4){
		pOAM[oamAddr] = value;
	}

	if(address == 0x5){
		if(w == 0){ // first write
			x = value & 0x07;
			t.coarseX = value >> 3;
			w = 1;
		} else {    // second write
			t.fineY = value & 0x7;
			t.coarseY = value >> 3;
			w = 0;
		}
	}

	if(address == 0x6){
		if(w == 0){ // first write
			t.reg = (uint16_t)((value & 0x3F) << 8) | (t.reg & 0x00FF);
			w = 1;
		} else {    // second write
			t.reg = (t.reg & 0xFF00) | value;
			v = t;
			w = 0;
		}
	}

	if(address == 0x7){
		ppuWrite(v.reg, value);
		v.reg += (ppuctrl.incrementMode ? 32 : 1);
	}
}

uint32_t PPU2C02::getColor(uint8_t palette, uint8_t pixel){
	return color[ppuRead(0x3F00 + (palette << 2) + pixel) & 0x3F];
};

void PPU2C02::clock(){
	auto incrementScrollX = [&](){
		if(ppumask.bgRender || ppumask.spriteRender){
			if(v.coarseX == 31){
				v.coarseX = 0;
				v.nametableX = ~v.nametableX;
			} else {
				v.coarseX++;
			}
		}
	};
	
	auto incrementScrollY = [&](){
		if(ppumask.bgRender || ppumask.spriteRender){
			if(v.fineY < 7){
				v.fineY++;
			} else {
				v.fineY = 0;
			
				if(v.coarseY == 29){
					v.coarseY = 0;
					v.nametableY = ~v.nametableY; 
				} else if(v.coarseY == 31){
					v.coarseY = 0;
				} else {
					v.coarseY++;
				}
			}
		}
	};
	
	auto transferAddressX = [&](){
		if(ppumask.bgRender || ppumask.spriteRender){
			v.nametableX = t.nametableX;
			v.coarseX = t.coarseX;
		}
	};
	
	auto transferAddressY = [&](){
		if(ppumask.bgRender || ppumask.spriteRender){
			v.fineY = t.fineY;
			v.nametableY = t.nametableY;
			v.coarseY = t.coarseY;
		}
	};
	
	auto loadBgShifters = [&](){
		bgShifterPatternLs = (bgShifterPatternLs & 0xFF00) | bgNextTileLs;
		bgShifterPatternMs = (bgShifterPatternMs & 0xFF00) | bgNextTileMs;

		bgShifterAttributeLs = (bgShifterAttributeLs & 0xFF00) | ((bgNextTileAttribute & 0b01) ? 0xFF : 0x00);
		bgShifterAttributeMs = (bgShifterAttributeMs & 0xFF00) | ((bgNextTileAttribute & 0b10) ? 0xFF : 0x00);
	};
	
	auto updateShifters = [&](){
		if(ppumask.bgRender){
			bgShifterPatternLs <<= 1;
			bgShifterPatternMs <<= 1;

			bgShifterAttributeLs <<= 1;
			bgShifterAttributeMs <<= 1;
		}

		if(ppumask.spriteRender && 1 <= cycle && cycle < 258){
			for(int i = 0; i < spriteCount; ++i){
				if(spriteScanline[i].x > 0){
					spriteScanline[i].x--;
				} else {
					spriteShifterPatternLs[i] <<= 1;
					spriteShifterPatternMs[i] <<= 1;
				}
			}
		}
	};

	if(-1 <= scanline && scanline <= 239){
		if(scanline == -1 && cycle == 1){
			ppustatus.vBlank = 0;
			ppustatus.spriteZeroHit = 0;
			ppustatus.spriteOverflow = 0;

			for(int i = 0; i < 8; ++i){
				spriteShifterPatternLs[i] = 0;
				spriteShifterPatternMs[i] = 0;
			}	
		}

		if(scanline == 0 && cycle == 0)
			cycle = 1;

		if((2 <= cycle && cycle < 258) || (321 <= cycle && cycle < 338)){
			updateShifters();

			if((cycle - 1) % 8 == 0){
				loadBgShifters();
				bgNextTileId = ppuRead(0x2000 | (v.reg & 0xFFF));
			}
			
			if((cycle - 1) % 8 == 2){
				bgNextTileAttribute = ppuRead(0x23C0 | (v.nametableY << 11) 
													| (v.nametableX << 10) 
													| ((v.coarseY >> 2) << 3)
													| (v.coarseX >> 2));

				if(v.coarseY & 0x2) bgNextTileAttribute >>= 4;
				if(v.coarseX & 0x2) bgNextTileAttribute >>= 2;
				bgNextTileAttribute &= 0x03;
			}
			
			if((cycle - 1) % 8 == 4){
				bgNextTileLs = ppuRead((ppuctrl.bgTile << 12) 
										+ ((uint16_t)bgNextTileId << 4)
										+ v.fineY);
			}
			
			if((cycle - 1) % 8 == 6){
				bgNextTileMs = ppuRead((ppuctrl.bgTile << 12)
										+ ((uint16_t)bgNextTileId << 4) 
										+ (v.fineY) + 8);
			}

			if((cycle - 1) % 8 == 7){
				incrementScrollX();
			}
		}

		if(cycle == 256){
			incrementScrollY();
		}

		if(cycle == 257){
			loadBgShifters();
			transferAddressX();
		}

		if(cycle == 338 || cycle == 340){
			bgNextTileId = ppuRead(0x2000 | (v.reg & 0xFFF));
		}

		if(scanline == -1 && 280 <= cycle && cycle < 305){
			transferAddressY();
		}

		if(cycle == 257 && 0 <= scanline){
			std::memset(spriteScanline, 0xFF, 8 * sizeof(spriteObject));
			spriteCount = 0;

			for(int i = 0; i < 8; ++i){
				spriteShifterPatternLs[i] = 0;
				spriteShifterPatternMs[i] = 0;
			}

			uint8_t nOAMEntry = 0;

			bSpriteZeroHitPossible = false;

			while(nOAMEntry < 64 && spriteCount < 9){
				int16_t diff = ((int16_t)scanline - (int16_t)oam[nOAMEntry].y);

				if(0 <= diff && diff < (ppuctrl.spriteHeight ? 16 : 8)){
					if(spriteCount < 8){
						if(nOAMEntry == 0){
							bSpriteZeroHitPossible = true;
						}

						memcpy(&spriteScanline[spriteCount], &oam[nOAMEntry], sizeof(spriteObject));
						spriteCount++;
					}
				}

				nOAMEntry++;
			}

			ppustatus.spriteOverflow = (spriteCount > 8);
		}
		
		if(cycle == 340){
			for(int i = 0; i < spriteCount; ++i){
				uint8_t spritePatternBitsLs, spritePatternBitsMs;
				uint16_t spritePatternAddrLs, spritePatternAddrMs;

				if(!ppuctrl.spriteHeight){
					if(!(spriteScanline[i].attribute & 0x80)){
						spritePatternAddrLs = (ppuctrl.spriteTile << 12) 
							| (spriteScanline[i].index << 4) 
							| (scanline - spriteScanline[i].y);
					} else {
						spritePatternAddrLs = (ppuctrl.spriteTile << 12)
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

				spritePatternBitsLs = ppuRead(spritePatternAddrLs);
				spritePatternBitsMs = ppuRead(spritePatternAddrMs);

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
			ppustatus.vBlank = 1;

			if(ppuctrl.nmiEnable)
				nmi = true;
		}
	}

	uint8_t bgPixel = 0;
	uint8_t bgPalette = 0;

	if(ppumask.bgRender){
		uint16_t bit_mux = 0x8000 >> x;

		uint8_t pixelLs = (bgShifterPatternLs & bit_mux) != 0;
		uint8_t pixelMs = (bgShifterPatternMs & bit_mux) != 0;
		bgPixel = (pixelMs << 1) | pixelLs;

		uint8_t palLs = (bgShifterAttributeLs & bit_mux) != 0;
		uint8_t palMs = (bgShifterAttributeMs & bit_mux) != 0;
		bgPalette = (palMs << 1) | palLs;
	}

	uint8_t fgPixel = 0;
	uint8_t fgPalette = 0;
	uint8_t fgPriority = 0;

	if(ppumask.spriteRender){
		bSpriteZeroBeingRendered = false;
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
			if(ppumask.bgRender & ppumask.spriteRender){
				if(!(ppumask.bgLeftmost | ppumask.spriteLeftmost)){
					if(9 <= cycle && cycle < 258){
						ppustatus.spriteZeroHit = 1;
					}
				} else {
					if(1 <= cycle && cycle < 258){
						ppustatus.spriteZeroHit = 1;
					}
				}
			}
		}
	}

	if(0 < scanline && scanline < 240 && 0 < (cycle - 1) && (cycle - 1) < 256){
		uint32_t pixelColor = getColor(palette, pixel);
		if(windowPixelColor[scanline * 256 + (cycle - 1)] != pixelColor){
			windowPixelColor[(scanline % 240) * 256 + (cycle - 1)] = pixelColor;
			updateScreen();
		}
	}

	cycle++;
	if(341 <= cycle){
		cycle = 0;
		scanline++;
			
		if(261 <= scanline){
			scanline = -1;
		}
	}
}
