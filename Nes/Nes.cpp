#include <iostream>
using namespace std;

#include <cstdint>


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
	Overflow = 6;
	Negative = 7;
}

class MOS_6502 {
	// CPU registers
	uint8_t a;   // Accumulator
	uint8_t x;   // Index X
	uint8_t y;   // Index Y
	uint16_t pc; // Program Counter
	uint8_t s;   // Stack Pointer
	
	uint8_t p;   // status register

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

	void instructions(opcode){
		// ADC - Add with Carry
		void adc(uint16_t address){
			uint8_t value = bus.getValue(address);
			
			uint8_t c = (a + value) > 255 ? 1 : 0;
			handleFlag(Carry, c == 1);

			a = a + value + c;

			//TODO: Overflow Flag

			handleFlag(Zero, a == 0);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		// Logical and
		void land(uint16_t address){
			int value = bus.getValue(address);
			a = a & value;

			handleFlag(Zero, a == 0);
			bool n = a & (1 << 7);
			handleFlag(Negative, n);
		}

		void asl(uint16_t address){
			uint8_t value = bus.getValue(address);
			
			uint8_t c = value & (1 << 7);
			handleFlag(Carry, c);

			a = a << 1;

			handleFlag(Zero, a == 0);

			bool n = (a & (1 << 7)) != 0;
			handleFlag(Negative, n);
		}

		void bcc(){ // TODO
			Bool carryFlag = getFlag(Carry);
			
			if(carryFlag){
				
			}else{

			}
		}

		void bcs(){ // TODO
			Bool carryFlag = getFlag(Carry);
			
			if(carryFlag){
				
			}else{

			}
		}

		void beq(){ // TODO
			
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

		// Branch if Minus
		void bmi(){

		}

		// Branch if Not Equal
		void bne(){

		}

		// Branch if Positive
		void bpl(){

		}

		// BRK - Force Interrupt
		if(opcode == 0x00){
			// TODO: cycles
		}

		// Branch if Overflow Clear
		void bvc(){

		}

		// Branch if Overflow Set
		void bvs(){

		}

		// Clear Carry Flag
		void clc{
			setC
		}

		// Clear Decimal Mode
		void cld(){
			
		}

		// Clear Interrupt Disable
		void cli(){

		}

		// Clear Overflow Flag
		void clv(){

		}

		// Compare
		void cmp(){

		}

		// Compare X Register
		void cpx(){

		}

		// Compare Y Register
		void cpy(){

		}

		// Decrement Memory
		void dec(){

		}

		// Decrement X Register
		void dex(){

		}

		// Decrement Y Register
		void dey(){

		}

		// Exclusive OR
		void eor(){

		}

		// Increment Memory
		void inc(){

		}

		// Increment X Register
		void inx(){

		}

		// Increment Y Register
		void iny(){

		}

		// Jump
		void jmp(){

		}

		// Jump to Subroutine
		void jsr(){

		}

		// Load Accumulator
		void lda(){

		}

		// Load X Register
		void ldx(){

		}

		// Load Y Register
		void ldy(){

		}

		// Logical Shift Right
		void lsr(){

		}

		// No Operation
		void nop(){

		}

		// Logical Inclusive OR
		void ora(){

		}

		// Push Accumulator
		void pha(){

		}

		// Push Processor Status
		void php(){

		}

		// Pull Accumulator
		void pla(){

		}

		// Pull Processor Status
		void plp(){

		}

		// Rotate Left
		void rol(){

		}

		// Rotate Right
		void ror(){

		}

		// Return from Interrupt
		void rti(){

		}

		// Return from Subroutine
		void rts(){

		}

		// Subtract with Carry
		void sbc(){

		}

		// Set Carry Flag
		void sec(){

		}

		// Set Decimal Flag
		void sed(){

		}

		// Set Interrupt Disable
		void sei(){

		}

		// Store Accumulator
		void sta(){

		}

		// Store X Register
		void stx(){

		}

		// Store Y Register
		void sty(){

		}

		// Transfer Accumulator to X
		void tax(){

		}

		// Transfer Accumulator to Y
		void tay(){

		}

		// Transfer Stack Pointer to X
		void tsx(){

		}

		// Transfer X to Accumulator
		void txa(){

		}

		// Transfer X to Stack Pointer
		void txs(){

		}

		// Transfer Y to Accumulator
		void tya(){
			handleFlag(Zero, a == 0);
			bool expression = (a & (1 << 7)) == 0; 
			handleFlag(Negative, expression);
		}

	}
};

MOS_6502 cpu;

int main() {
	return 0;
}
