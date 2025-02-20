#include "cpu6502.h"
#include "ppu2C02.h"
#include "bus.h"
#include "window.h"

#include <sstream>
#include <string>
#include <bitset>

#include <iomanip> 

using namespace std;

int main() {
	Bus bus;
	bus.cpu.connectBus(&bus);
	bus.loadCartridge();
	bus.reset();
	bus.controller[0] = 0x80;
	bus.clock();

	return 0;
}
