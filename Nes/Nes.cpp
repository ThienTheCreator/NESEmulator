#include <iostream>
#include <cstdint>

using namespace std;

class Bus{
	uint8_t memory[65536];
public:
	void setValue(uint16_t address, uint8_t value){
		memory[address] = value;
	};

	uint8_t getValue(uint16_t address){
		return memory[address];
	};
};

Bus bus;
	
/* 
7  bit  0
---- ----
NV1B DIZC
|||| ||||
|||| |||+- Carry
|||| ||+-- Zero
|||| |+--- Interrupt Disable
|||| +---- Decimal
|||+------ (No CPU effect; see: the B flag)
||+------- (No CPU effect; always pushed as 1)
|+-------- Overflow
V+--------- Negative

Diagram from https://www.nesdev.org/wiki/Status_flags
*/
enum flag{
	Carry = 0,
	Zero = 1,
	Interrupt = 2,
	Decimal = 3,
	Break = 4,
	Overflow = 6,
	Negative = 7
};

class MOS_6502 {
public:
	// CPU registers
	uint8_t  a  = 0;    // Accumulator
	uint8_t  x  = 0;    // Index X
	uint8_t  y  = 0;    // Index Y
	uint16_t pc = 0;    // Program Counter
	uint8_t  s  = 0xFF; // Stack Pointer
	
	uint8_t p = 0b01100000; // status register

	void setFlag(uint8_t bit){
		uint8_t value = 1 << bit;
		p = p | value;
	}

	void unsetFlag(uint8_t bit){
		uint8_t value = 0b11111111 ^ (1 << bit);
		p = p & value;
	}

	void handleFlag(uint8_t flag, bool expression){
		if(expression) {
			setFlag(flag);
		} else {
			unsetFlag(flag);
		}
	}

	bool getFlag(uint8_t bit){
		if(p & (1 << bit)){
			return true;
		}else{
			return false;
		}
	}

		/* 
		** Load or Store Operations 
		*/

		// Load Accumulator
		void lda(uint16_t address){
			a = bus.getValue(address);
			
			bool z = a == 0;
			handleFlag(Zero, z);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Load X Register
		void ldx(uint16_t address){
			x = bus.getValue(address);

			bool z = x == 0;
			handleFlag(Zero, z);

			bool n = (x & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Load Y Register
		void ldy(uint16_t address){
			y = bus.getValue(address);

			bool z = y == 0;
			handleFlag(Zero, z);

			bool n = (y & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Store Accumulator
		void sta(uint16_t address){
			bus.setValue(address, a);
		}

		// Store X Register
		void stx(uint16_t address){
			bus.setValue(address, x);
		}

		// Store Y Register
		void sty(uint16_t address){
			bus.setValue(address, y);
		}

		/*
		** Register Transfers
		*/

		// Transfer Accumulator to X
		void tax(){
			x = a;

			bool z = x == 0;
			handleFlag(Zero, z);

			bool n = (x & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Transfer Accumulator to Y
		void tay(){
			y = a;

			bool z = y == 0;
			handleFlag(Zero, z);

			bool n = (y & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Transfer X to Accumulator
		void txa(){
			a = x;

			bool z = a == 0;
			handleFlag(Zero, z);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Transfer Y to Accumulator
		void tya(){
			handleFlag(Zero, a == 0);
			bool expression = (a & (1 << 7)) == 0; 
			handleFlag(Negative, expression);
		}

		/*
		** Stack Operation
		*/

		// Transfer Stack Pointer to X
		void tsx(){
			x = s;

			bool z = x == 0;
			handleFlag(Zero, x);

			bool n = x & (1 << 7);
			handleFlag(Negative, n);
		}

		// Transfer X to Stack Pointer
		void txs(){
			s = x;
		}

		// Push Accumulator
		void pha(){
			bus.setValue(s, a);
			s--;
		}

		// Push Processor Status
		void php(){
			bus.setValue(s, p);
			s--;
		}

		// Pull Accumulator
		void pla(){
			a = bus.getValue(s + 1);
			s++;

			bool z = a == 0;
			handleFlag(Zero, z);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Pull Processor Status
		void plp(){
			p = bus.getValue(s + 1);
			s++;
		}

		/*
		** Logical 
		*/

		// Logical and
		void andL(uint16_t address){
			int value = bus.getValue(address);
			a = a & value;

			handleFlag(Zero, a == 0);
			bool n = a & (1 << 7);
			handleFlag(Negative, n);
		}
		
		// Exclusive OR
		void eor(uint16_t address){
			uint8_t value = bus.getValue(address);
			a = a ^ value;

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
			
			bool z = a == 0;
			handleFlag(Zero, z);
		}

		// Logical Inclusive OR
		void ora(uint16_t address){
			uint8_t m = bus.getValue(address);
			a = a | m;

			bool c = a == 0;
			handleFlag(Carry, a == 0);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// bit test
		void bit(uint16_t address){
			uint8_t value = bus.getValue(address);
			
			bool n = (value & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool v = (value & (1 << 6)) != 0;
			handleFlag(Overflow, v);

			bool z = (a & value) == 0;
			handleFlag(Zero, z);
		}

		/*
		** Arithmetic
		*/

		// ADC - Add with Carry
		void adc(uint16_t address){
			uint8_t value = bus.getValue(address);

			bool a7 = (a & (1 << 7)) != 0;
			bool m7 = (value & (1 << 7)) != 0;

			bool carryIn = getFlag(Carry);

			a = a + value + carryIn;
			bool carryOut = a < value;
			handleFlag(Carry, carryOut);

			handleFlag(Zero, a == 0);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool signResult = (a & (1 << 7)) != 0;
			bool v = (a7 == m7) && (a7 != signResult);
			handleFlag(Overflow, v);
		}

		// Subtract with Carry
		void sbc(uint16_t address){
			uint8_t m = bus.getValue(address);
			bool c = getFlag(Carry);
			a = a - m - (1 - c);

		}

		// Compare
		void cmp(uint16_t address){
			uint8_t value = bus.getValue(address);

			uint8_t cmpValue = a - value;

			bool c = a >= value;
			handleFlag(Carry, c);

			bool z = a == value;
			handleFlag(Zero, z);

			bool n = (cmpValue & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Compare X Register
		void cpx(uint16_t address){
			uint8_t value = bus.getValue(address);
			uint8_t result = x - value;

			bool c = x >= value;
			handleFlag(Carry, c);

			bool z = x == value;
			handleFlag(Zero, z);

			bool n = (result & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Compare Y Register
		void cpy(uint16_t address){
			uint8_t value = bus.getValue(address);
			uint8_t result = y - value;

			bool c = y >= value;
			handleFlag(Carry, c);

			bool z = y == value;
			handleFlag(Zero, z);

			bool n = (result & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		/*
		** Increment and Decrements
		*/

		// Increment Memory
		void inc(uint16_t address){
			uint8_t value = bus.getValue(address);
			value = value + 1;

			bus.setValue(address, value);

			bool n = (value & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = value == 0;
			handleFlag(Zero, z);
		}

		// Increment X Register
		void inx(){
			x = x + 1;

			bool n = (x & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = x == 0;
			handleFlag(Zero, z);

		}

		// Increment Y Register
		void iny(){
			y = y + 1;

			bool n = (y & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = y == 0;
			handleFlag(Zero, z);
		}

		// Decrement Memory
		void dec(uint16_t address){
			uint8_t value = bus.getValue(address);
			value = value - 1;

			bus.setValue(address, value);

			bool n = (value & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = value == 0;
			handleFlag(Zero, z);
		}

		// Decrement X Register
		void dex(){
			x = x - 1;

			bool n = (x & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = x == 0;
			handleFlag(Zero, z);
		}

		// Decrement Y Register
		void dey(){
			y = y - 1;

			bool n = (y & (1 << 7)) != 0;
			handleFlag(Negative, n);

			bool z = y == 0;
			handleFlag(Zero, z);
		}

		/*
		** Shifts
		*/

		// Arithmetic Shift Left
		void asl(uint16_t address){
			uint8_t value = bus.getValue(address);
			
			uint8_t c = value & (1 << 7);
			handleFlag(Carry, c);

			a = a << 1;

			bool z = a == 0;
			handleFlag(Zero, z);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Logical Shift Right
		void lsr(){
			uint8_t tempValue;

			if(1){ // TODO
				tempValue = a;
			}else{
				// tempValue = m;
			}
			
			bool c = tempValue & 1;
			handleFlag(Carry, c);
			
			tempValue = tempValue >> 1;

			bool z = tempValue == 0;
			handleFlag(Zero, z);

			bool v = tempValue & (1 << 7);
			handleFlag(Overflow, v);

			if(1){ // TODO
				a = tempValue;
			}else{
				// m = tempValue;
			}
		}

		// Rotate Left
		void rol(){
			bool bitSeven = getFlag(7);
			
			// TODO
			a << 1;
			// m << 1;

			bool carryFlag = getFlag(Carry);
			
			// TODO
			a = a | carryFlag;
			// m = m | carryFlag;

			handleFlag(Carry, bitSeven);
		}

		// Rotate Right
		void ror(){
			bool bitZero = getFlag(0);
			
			// TODO
			a >> 1;
			// m >> 1;

			bool carryFlag = getFlag(Carry);
			
			// TODO
			a = a | (carryFlag << 7);
			// m = m | (carryFlag << 7);

			handleFlag(Carry, bitZero);
		}

		/*
		** Jumps and Calls
		*/

		// Jump
		void jmp(uint16_t address){
			pc = address;
		}

		// Jump to Subroutine
		void jsr(uint16_t address){
			uint16_t tempAddress = s | 0x100;
			bus.setValue(tempAddress, pc - 1);
			s--;
			pc = address;
		}

		// Return from Subroutine
		void rts(){
			pc = bus.getValue(s | 0x100);
		}

		/*
		** Branches
		*/

		// Branch if Carry Clear
		void bcc(uint16_t displacement){ // TODO
			bool carryFlag = getFlag(Carry);
			
			if(!carryFlag){
				pc += displacement;
			}
		}

		// Branch if Carry Set
		void bcs(uint16_t displacement){ // TODO
			bool carryFlag = getFlag(Carry);
			
			if(carryFlag){
				pc += displacement;	
			}
		}

		// Branch if Equal
		void beq(uint16_t displacement){ // TODO
			bool zeroFlag = getFlag(Zero);
			
			if(zeroFlag){
				pc += displacement;
			}
		}


		// Branch if Minus
		void bmi(uint16_t displacement){
			bool negativeFlag = getFlag(Negative);
			
			if(negativeFlag){
				pc += displacement;
			}
		}

		// Branch if Not Equal
		void bne(uint16_t displacement){
			bool zeroFlag = getFlag(Zero);
			
			if(!zeroFlag){
				pc += displacement;
			}
		}

		// Branch if Positive
		void bpl(uint16_t displacement){
			bool negativeFlag = getFlag(Negative);
			
			if(!negativeFlag){
				pc += displacement;
			}
		}

		// Branch if Overflow Clear
		void bvc(uint16_t displacement){
			bool overflowFlag = getFlag(Overflow);
			
			if(!overflowFlag){
				pc += displacement;
			}
		}

		// Branch if Overflow Set
		void bvs(uint16_t displacement){
			bool overflowFlag = getFlag(Overflow);
			
			if(overflowFlag){
				pc += displacement;
			}
		}

		/*
		** Status Flag Changes
		*/

		// Clear Carry Flag
		void clc(){
			handleFlag(Carry, false);
		}

		// Clear Decimal Mode
		void cld(){
			handleFlag(Decimal, false);	
		}

		// Clear Interrupt Disable
		void cli(){
			handleFlag(Interrupt, false);
		}

		// Clear Overflow Flag
		void clv(){
			handleFlag(Overflow, false);
		}

		// Set Carry Flag
		void sec(){
			handleFlag(Carry, true);
		}

		// Set Decimal Flag
		void sed(){
			handleFlag(Decimal, true);
		}

		// Set Interrupt Disable
		void sei(){
			handleFlag(Interrupt, true);
		}
		
		/*
		** System Functions
		*/

		// Force an Interrupt
		void brk(){
			bus.setValue(s | 0x100, pc);
			s--;
			bus.setValue(s | 0x100, p);
			s--;

			uint16_t tempValue = bus.getValue(0xFFFE);
			tempValue << 8;
			tempValue = tempValue | bus.getValue(0xFFFF);

			handleFlag(Break, true);
		}

		// No Operation
		void nop(){

		}
		
		// Return from Interrupt
		void rti(){
			p = bus.getValue((s + 1) | 0x100);
			s++;
			pc = bus.getValue((s + 1) | 0x100);
			s++;
		}

	void executeTest(uint8_t opcode){
		if(opcode == 0x78){
			sei();
		}
		if(opcode == 0x58){
			cld();
		}
		pc++;
	}
};

MOS_6502 cpu;

#include<fstream>
#include<sstream>
#include<string>

int main() {
	ifstream myfile("donkey kong.nes");
	char c;
	for(int i = 0; i < 16; i++){
		myfile.get(c);
		cout << (int)c << endl;
	}

	for(int i = 0; i < 16384; i++){
		myfile.get(c);
		bus.setValue(0x8000 + i, c);
		bus.setValue(0xC000 + i, c);
	}

	myfile.close();

	uint16_t test = bus.getValue(0xFFFD) << 8;
	test |= bus.getValue(0xFFFC);
	
	cout << hex << test << endl;

	cpu.pc = test;

	return 0;
}
