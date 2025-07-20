#pragma once

#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "window.h"

class PPU2C02{	
	uint32_t color[64];

	uint8_t patternTable[2][4096];
	uint8_t nameTable[4][1024];
	uint8_t paletteTable[32];

	uint8_t ppuGenLatch = 0;

	union loopyRegister{
		struct{
			uint16_t coarseX :5;
			uint16_t coarseY :5;
			uint16_t nametableX: 1;
			uint16_t nametableY: 1;
			uint16_t fineY : 3;
			uint16_t unused: 1;
		};
		uint16_t reg;
	} v, t;

	uint8_t x: 3;
	uint8_t w: 1;

	uint8_t ppuDataBuffer = 0;
	
	uint8_t bgNextTileId = 0;
	uint8_t bgNextTileAttribute = 0;
	uint8_t bgNextTileLs = 0; 			// Least Significant
	uint8_t bgNextTileMs = 0; 			// Most Significant

	uint16_t bgShifterPatternLs = 0; 	// Pattern Least Significant
	uint16_t bgShifterPatternMs = 0; 	// Pattern Most Significant
	uint16_t bgShifterAttributeLs = 0; 	// Attribute Shifter Least Significant
	uint16_t bgShifterAttributeMs = 0;  // Attribute Shifter Least Significatn

	uint8_t spriteShifterPatternLs[8];
	uint8_t spriteShifterPatternMs[8];

	bool bSpriteZeroHitPossible = false;
	bool bSpriteZeroBeingRendered = false;

	int16_t scanline = 0;
	int16_t cycle = 0;
	bool oddFrame = false;

	HANDLE thread = CreateThread(NULL, 0, ep, NULL, 0, NULL);
public:
	PPU2C02();
	
	union PPUCTRL{
		struct {
			uint8_t nametableX: 1; 		// (0 = $2000; 1 = $2400;
			uint8_t nametableY: 1; 		//  2 = $2800; 3 = $2C00)
			uint8_t incrementMode: 1; 	// (0: add 1, going across; 1: add 32, going down)
			uint8_t spriteTile: 1; 		// (0: $0000; 1: $1000; ignored in 8x16 mode)
			uint8_t bgTile: 1; 			// (0: $0000; 1: $1000)
			uint8_t spriteHeight: 1; 	// (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
			uint8_t ppuSelect: 1; 		// (0: read backdrop from EXT pins; 1: output color on EXT pins)
			uint8_t nmiEnable: 1; 		// (0: off, 1: on)
		};
		uint8_t reg;
	} ppuctrl;

	union PPUMASK{
		struct{
			uint8_t greyscale:1;    	// (0: normal color, 1: greyscale)
			uint8_t bgLeftmost:1; 		// Bg leftmost 8 pixels of screen
			uint8_t spriteLeftmost:1; 	// sprite leftmost 8 pixels of screen
			uint8_t bgRender:1; 		// enable Bg rendering
			uint8_t spriteRender:1;		// enable sprite rendering
			uint8_t red:1; 				// emphasize red (green on PAL/Dendy)
			uint8_t green:1; 			// emphasize green (red on PAL/Dendy)
			uint8_t	blue:1; 			// emphasize blue
		};
		uint8_t reg;
	} ppumask;

	union PPUSTATUS{
		struct{
			uint8_t openBus:5;
			uint8_t spriteOverflow:1;
			uint8_t spriteZeroHit:1;
			uint8_t vBlank:1; 			// cleared on read. Unreliable;
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
	
	uint8_t oamAddr = 0;
	uint8_t oamData = 0;
	uint8_t ppuScroll = 0;
	uint8_t ppuAddr = 0;
	uint8_t ppuData = 0;
	uint8_t oamDma = 0;
	
	uint32_t getColor(uint8_t palette, uint8_t pixel);

	uint8_t ppuRead(uint16_t address);
	void ppuWrite(uint16_t address, uint8_t value);
 
	uint8_t cpuRead(uint16_t address);
	void cpuWrite(uint16_t address, uint8_t value);

	void clock();
	void reset();
	
	bool nmi = false;
};
