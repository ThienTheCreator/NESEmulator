#include "cpu6502.h"
#include "ppu2C02.h"
#include "bus.h"
#include "window.h"

#include <sstream>
#include <string>
#include <bitset>

#include <iomanip> 

using namespace std;

Bus bus;

int main() {
	bus.cpu.connectBus(&bus);
	bus.loadCartridge();
	bus.clock();

	return 0;
}
