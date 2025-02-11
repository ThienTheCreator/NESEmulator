#pragma once

#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "window.h"

class PPU2C02{
	uint32_t color[64];
	static const uint16_t width = 256;
	static const uint16_t height = 240;
	uint32_t screen[width * height];

	uint8_t patternTable[2][4096];
	uint8_t nameTable[4][1024];
	uint8_t paletteTable[32];

	uint8_t ppuGenLatch = 0;
	uint8_t ppuDataBuffer = 0;


	union loopyRegister{
		struct{
			uint8_t coarseX :5;
			uint8_t coarseY :5;
			uint8_t nametable: 2;
			uint8_t fineY : 3;
			uint8_t unused: 1;
		};
		uint16_t reg;
	} v, t;

	uint8_t x: 3;
	uint8_t w: 1;

	uint8_t nextTileId = 0;
	uint8_t nextTileAttribute = 0;
	uint8_t nextTileBgLs = 0;
	uint8_t nextTileBgMs = 0;

	uint16_t bgShifterPatternLs = 0;
	uint16_t bgShifterPatternMs = 0;
	uint16_t bgShifterAttributeLs = 0;
	uint16_t bgShifterAttributeMs = 0;

	uint8_t spriteShifterPatternLs[8];
	uint8_t spriteShifterPatternMs[8];

	bool bSpriteZeroHitPossible = false;
	bool bSpriteZeroBeingRendered = false;

	uint16_t scanline = 0;
	uint16_t cycle = 0;

	HANDLE thread = CreateThread(NULL, 0, ep, NULL, 0, NULL);
public:
	union PPUCTRL{
		struct {
			uint8_t nn: 2; // nametable x, y
			uint8_t  i: 1; // increment mode
			uint8_t  s: 1; // sprite tile select: unimplemented for 8x8 sprite	
			uint8_t  b: 1; // background tile select
			uint8_t  h: 1; // sprite height
			uint8_t  p: 1; // PPU master/slave
			uint8_t  v: 1; // nmi enable
		};
		uint8_t reg;
	} ppuctrl;

	union PPUMASK{
		struct{
			uint8_t Greyscale:1;
			uint8_t m:1;
			uint8_t M:1;
			uint8_t bg:1;
			uint8_t s:1;			
			uint8_t R:1;
			uint8_t G:1;
			uint8_t	B:1;
		};
		uint8_t reg;
	} ppumask;

	union PPUSTATUS{
		struct{
			uint8_t openBus:5;			
			uint8_t O:1; // sprite overflow
			uint8_t S:1; // sprite 0 hit
			uint8_t V:1; // vblank
		};
		uint8_t reg;
	} ppustatus;

	struct spriteObject{
		uint8_t y;
		uint8_t index;
		uint8_t attribute;
		uint8_t x;
	} oam[64];

	spriteObject spriteScanline[8];

	uint8_t spriteCount; 

	uint8_t* pOAM = (uint8_t*)oam;
	
	uint8_t oamAddr;
	uint8_t oamData;
	uint8_t ppuScroll;
	uint8_t ppuAddr;
	uint8_t ppuData;
	uint8_t oamDma;
	
	bool nmi = false;

	PPU2C02();

	void setupColor();
	uint32_t getColor(uint8_t palette, uint8_t pixel);

	uint8_t getValue(uint16_t address);
	void setValue(uint16_t address, uint8_t value);
 
	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

	void reset();
	void loadBgShifter();
	void updateShifter();
	void clock();
};
