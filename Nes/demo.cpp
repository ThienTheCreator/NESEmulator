#include "cpu6502.h"
#include "ppu2C02.h"
#include "bus.h"

#include <sstream>
#include <string>
#include <bitset>

#include <iomanip> 

using namespace std;

Bus bus;

int main() {
	bus.cpu.connectBus(&bus);

	uint16_t test = bus.cpuRead(0xfffc);
	test |= bus.cpuRead(0xfffd) << 8;

	cout << hex << (int)test << endl;

	bus.cpu.pc = test;

	int temp = 0;
	while(temp < 16384){
		if(1){
			/*
			cout << uppercase <<hex;
			cout << setw(4) << bus.cpu.pc;
			cout << " A:" << setfill('0') << setw(2) << (int)bus.cpu.a;
			cout <<	" X:" << setfill('0') << setw(2) << (int)bus.cpu.x;
			cout << " Y:" << setfill('0') << setw(2) << (int)bus.cpu.y; 
			cout << " P:" << setfill('0') << setw(2) << (int)bus.cpu.p;
			cout << " SP:" << setfill('0') << setw(2) << (int)bus.cpu.s;
			cout << endl;
			*/
			bus.cpu.executeInstruction(bus.cpuRead(bus.cpu.pc));
		}
		if(1){
			
		}
		temp++;
	}

	return 0;
}
