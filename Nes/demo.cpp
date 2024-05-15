#include "cpu6502.h"
#include "ppu.h"
#include "bus.h"

#include<fstream>
#include<sstream>
#include<string>

using namespace std;

Bus bus;
CPU6502 cpu; 

int main() {
	ifstream myfile("donkey kong.nes");
	char c;
	for(int i = 0; i < 16; i++){
		myfile.get(c);
	}

	for(int i = 0; i < 16384; i++){
		myfile.get(c);
		bus.setValue(0x8000 + i, c);
		bus.setValue(0xc000 + i, c);
	}

	myfile.close();

	cpu.connectBus(&bus);

	uint16_t test = bus.getValue(0xfffc);
	test |= bus.getValue(0xfffd) << 8;

	cpu.pc = test;

	cpu.executeTest(0x00);

	/*
	int temp = 0;
	while(temp < 4){
		cpu.executeTest(bus.getValue(cpu.pc));
		cout << cpu.pc << endl;
		temp++;
	}
	*/

	return 0;
}
