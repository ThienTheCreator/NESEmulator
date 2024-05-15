#pragma once

#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>  

#include "window.h"

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
	uint8_t OAM [256];
	bool NMI_occurrred;
public:
	void setupColor();

	void reset();

	void nmi(); 
};
