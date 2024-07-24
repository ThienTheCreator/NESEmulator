#pragma once

#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "window.h"

class PPU2C02{
	uint32_t color[64];

	uint8_t patternTable[2][4096];
	uint8_t nameTable[4][1024];

	struct OAM{
		uint8_t spriteY;
		uint8_t spriteTitle;
		uint8_t spriteAttribute;
		uint8_t SpriteX;
	} oam[64];

	uint8_t OAM [256];

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

	uint8_t shiftAttributeLs = 0;
	uint8_t shiftAttributeMs = 0;
	uint16_t shiftPatternLs = 0;
	uint16_t shiftPatternMs = 0;

	uint16_t scanline = 0;
	uint16_t cycle = 0;
public:
	union PPUCTRL{
		struct {
			uint8_t nn: 2;
			uint8_t  i: 1;
			uint8_t  s: 1; // unimplemented for 8x8 sprite	
			uint8_t  b: 1;
			uint8_t  h: 1;
			uint8_t  p: 1;
			uint8_t  v: 1;
		};
		uint8_t reg;
	} control;

	union PPUMASK{
		struct{
			uint8_t Greyscale:1;
			uint8_t m:1;
			uint8_t M:1;
			uint8_t b:1;
			uint8_t s:1;			
			uint8_t R:1;
			uint8_t G:1;
			uint8_t	B:1;
		};
		uint8_t reg;
	} mask;

	union PPUSTATUS{
		struct{
			uint8_t openBus:5;			
			uint8_t O:1;
			uint8_t S:1;
			uint8_t V:1;
		};
		uint8_t reg;
	} status;

	uint8_t oamaddr;
	uint8_t oamdata;
	uint8_t ppuscroll;
	uint8_t ppudata;
	uint8_t oamdma;
	uint8_t ppuaddr;

	PPU2C02();

	void setupColor();

	uint8_t getValue(uint16_t address);
	void setValue(uint16_t address, uint8_t value);

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t value);

	void reset();

	void executePPU();
};
