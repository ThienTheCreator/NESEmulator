#include <iostream>
#include <cstdint>
#include <stdio.h>

using namespace std;

class PPU{
	uint32_t color[64];
public:
	void setupColor(){


		for(int i = 0; i < 3; i++){
			cout << (int)color[0][i] << endl;
		}
	}
};

PPU ppu_2C02;

int main() {
	ppu_2C02.setupColor();
	getchar();
	return 0;
}
