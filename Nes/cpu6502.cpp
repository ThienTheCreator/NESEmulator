#include <iostream>
#include <cstdint>

#include "bus.h"
#include "cpu6502.h"

using namespace std;

class Bus;

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


void CPU6502::setFlag(uint8_t bit){
	uint8_t value = 1 << bit;
	p = p | value;
}

void CPU6502::unsetFlag(uint8_t bit){
	uint8_t value = 0b11111111 ^ (1 << bit);
	p = p & value;
}

void CPU6502::handleFlag(uint8_t flag, bool expression){
	if(expression) {
		setFlag(flag);
	} else {
		unsetFlag(flag);
	}
}

bool CPU6502::getFlag(uint8_t bit){
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
void CPU6502::lda(uint16_t value){
	a = value;
	
	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Load X Register
void CPU6502::ldx(uint16_t address){
	x = bus->getValue(address);

	bool z = x == 0;
	handleFlag(Zero, z);

	bool n = (x & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Load Y Register
void CPU6502::ldy(uint16_t address){
	y = bus->getValue(address);

	bool z = y == 0;
	handleFlag(Zero, z);

	bool n = (y & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Store Accumulator
void CPU6502::sta(uint16_t address){
	bus->setValue(address, a);
}

// Store X Register
void CPU6502::stx(uint16_t address){
	bus->setValue(address, x);
}

// Store Y Register
void CPU6502::sty(uint16_t address){
	bus->setValue(address, y);
}

/*
** Register Transfers
*/

// Transfer Accumulator to X
void CPU6502::tax(){
	x = a;

	bool z = x == 0;
	handleFlag(Zero, z);

	bool n = (x & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Transfer Accumulator to Y
void CPU6502::tay(){
	y = a;

	bool z = y == 0;
	handleFlag(Zero, z);

	bool n = (y & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Transfer X to Accumulator
void CPU6502::txa(){
	a = x;

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Transfer Y to Accumulator
void CPU6502::tya(){
	handleFlag(Zero, a == 0);
	bool expression = (a & (1 << 7)) == 0; 
	handleFlag(Negative, expression);
}

/*
** Stack Operation
*/

// Transfer Stack Pointer to X
void CPU6502::tsx(){
	x = s;

	bool z = x == 0;
	handleFlag(Zero, x);

	bool n = x & (1 << 7);
	handleFlag(Negative, n);
}

// Transfer X to Stack Pointer
void CPU6502::txs(){
	s = x;
}

// Push Accumulator
void CPU6502::pha(){
	bus->setValue(s, a);
	s--;
}

// Push Processor Status
void CPU6502::php(){
	bus->setValue(s, p);
	s--;
}

// Pull Accumulator
void CPU6502::pla(){
	a = bus->getValue(s + 1);
	s++;

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Pull Processor Status
void CPU6502::plp(){
	p = bus->getValue(s + 1);
	s++;
}

/*
** Logical 
*/

// Logical and
void CPU6502::andL(uint16_t address){
	int value = bus->getValue(address);
	a = a & value;

	handleFlag(Zero, a == 0);
	bool n = a & (1 << 7);
	handleFlag(Negative, n);
}

// Exclusive OR
void CPU6502::eor(uint16_t address){
	uint8_t value = bus->getValue(address);
	a = a ^ value;

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
	
	bool z = a == 0;
	handleFlag(Zero, z);
}

// Logical Inclusive OR
void CPU6502::ora(uint16_t address){
	uint8_t m = bus->getValue(address);
	a = a | m;

	bool c = a == 0;
	handleFlag(Carry, a == 0);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// bit test
void CPU6502::bit(uint16_t address){
	uint8_t value = bus->getValue(address);
	
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
void CPU6502::adc(uint16_t address){
	uint8_t value = bus->getValue(address);

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
void CPU6502::sbc(uint16_t address){
	uint8_t m = bus->getValue(address);
	bool c = getFlag(Carry);
	a = a - m - (1 - c);

}

// Compare
void CPU6502::cmp(uint16_t address){
	uint8_t value = bus->getValue(address);

	uint8_t cmpValue = a - value;

	bool c = a >= value;
	handleFlag(Carry, c);

	bool z = a == value;
	handleFlag(Zero, z);

	bool n = (cmpValue & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Compare X Register
void CPU6502::cpx(uint16_t address){
	uint8_t value = bus->getValue(address);
	uint8_t result = x - value;

	bool c = x >= value;
	handleFlag(Carry, c);

	bool z = x == value;
	handleFlag(Zero, z);

	bool n = (result & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Compare Y Register
void CPU6502::cpy(uint16_t address){
	uint8_t value = bus->getValue(address);
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
void CPU6502::inc(uint16_t address){
	uint8_t value = bus->getValue(address);
	value = value + 1;

	bus->setValue(address, value);

	bool n = (value & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool z = value == 0;
	handleFlag(Zero, z);
}

// Increment X Register
void CPU6502::inx(){
	x = x + 1;

	bool n = (x & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool z = x == 0;
	handleFlag(Zero, z);

}

// Increment Y Register
void CPU6502::iny(){
	y = y + 1;

	bool n = (y & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool z = y == 0;
	handleFlag(Zero, z);
}

// Decrement Memory
void CPU6502::dec(uint16_t address){
	uint8_t value = bus->getValue(address);
	value = value - 1;

	bus->setValue(address, value);

	bool n = (value & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool z = value == 0;
	handleFlag(Zero, z);
}

// Decrement X Register
void CPU6502::dex(){
	x = x - 1;

	bool n = (x & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool z = x == 0;
	handleFlag(Zero, z);
}

// Decrement Y Register
void CPU6502::dey(){
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
void CPU6502::asl(uint16_t address){
	uint8_t value = bus->getValue(address);
	
	uint8_t c = value & (1 << 7);
	handleFlag(Carry, c);

	a = a << 1;

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Logical Shift Right
void CPU6502::lsr(){
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
void CPU6502::rol(){
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
void CPU6502::ror(){
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
void CPU6502::jmp(uint16_t address){
	pc = address;
}

// Jump to Subroutine
void CPU6502::jsr(uint16_t address){
	uint16_t tempAddress = s | 0x100;
	bus->setValue(tempAddress, pc - 1);
	s--;
	pc = address;
}

// Return from Subroutine
void CPU6502::rts(){
	pc = bus->getValue(s | 0x100);
}

/*
** Branches
*/

// Branch if Carry Clear
void CPU6502::bcc(uint16_t displacement){ // TODO
	bool carryFlag = getFlag(Carry);
	
	if(!carryFlag){
		pc += displacement;
	}
}

// Branch if Carry Set
void CPU6502::bcs(uint16_t displacement){ // TODO
	bool carryFlag = getFlag(Carry);
	
	if(carryFlag){
		pc += displacement;	
	}
}

// Branch if Equal
void CPU6502::beq(uint16_t displacement){ // TODO
	bool zeroFlag = getFlag(Zero);
	
	if(zeroFlag){
		pc += displacement;
	}
}


// Branch if Minus
void CPU6502::bmi(uint16_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(negativeFlag){
		pc += displacement;
	}
}

// Branch if Not Equal
void CPU6502::bne(uint16_t displacement){
	bool zeroFlag = getFlag(Zero);
	
	if(!zeroFlag){
		pc += displacement;
	}
}

// Branch if Positive
void CPU6502::bpl(uint16_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(!negativeFlag){
		pc += displacement;
	}
}

// Branch if Overflow Clear
void CPU6502::bvc(uint16_t displacement){
	bool overflowFlag = getFlag(Overflow);
	
	if(!overflowFlag){
		pc += displacement;
	}
}

// Branch if Overflow Set
void CPU6502::bvs(uint16_t displacement){
	bool overflowFlag = getFlag(Overflow);
	
	if(overflowFlag){
		pc += displacement;
	}
}

/*
** Status Flag Changes
*/

// Clear Carry Flag
void CPU6502::clc(){
	handleFlag(Carry, false);
}

// Clear Decimal Mode
void CPU6502::cld(){
	handleFlag(Decimal, false);	
}

// Clear Interrupt Disable
void CPU6502::cli(){
	handleFlag(Interrupt, false);
}

// Clear Overflow Flag
void CPU6502::clv(){
	handleFlag(Overflow, false);
}

// Set Carry Flag
void CPU6502::sec(){
	handleFlag(Carry, true);
}

// Set Decimal Flag
void CPU6502::sed(){
	handleFlag(Decimal, true);
}

// Set Interrupt Disable
void CPU6502::sei(){
	handleFlag(Interrupt, true);
}

/*
** System Functions
*/

// Force an Interrupt
void CPU6502::brk(){
	bus->setValue(s | 0x100, pc);
	s--;
	bus->setValue(s | 0x100, p);
	s--;

	uint16_t tempValue = bus->getValue(0xFFFE);
	tempValue << 8;
	tempValue = tempValue | bus->getValue(0xFFFF);

	handleFlag(Break, true);
}

// No Operation
void CPU6502::nop(){

}

// Return from Interrupt
void CPU6502::rti(){
	p = bus->getValue((s + 1) | 0x100);
	s++;
	pc = bus->getValue((s + 1) | 0x100);
	s++;
}

void CPU6502::irq(){
	if(getFlag(Interrupt) == false){
		brk();		
	}
}

void CPU6502::nmi(){

}

enum addressingMode{
	Implicit = 8,
	Accumulator = 9,
	Immediate = 10,
	ZeroPage = 11,
	ZeroPageX = 12,
	ZeroPageY = 13,
	Relative = 14,
	Absolute = 15,
	AbsoluteX = 16,
	AbsoluteY = 17,
	Indirect = 18,
	IndexedIndirect = 19,
	IndirectIndexed = 20
};

uint16_t CPU6502::handleMode(int mode, uint16_t value){
	p++;

	if(mode == 10){ // Immediate
		return bus->getValue(pc); 
	}

	if(mode == 11){ // Zero Page
		return bus->getValue(value);
	}

	if(mode == 12){ // Zero Page X
		return bus->getValue((uint8_t)(value + x));
	}

	if(mode == 15){ // Absolute
		uint16_t address = bus->getValue(pc) | (bus->getValue(++pc) << 8);
		return bus->getValue(address);
	}

	if(mode == 16){ // Absolute X
	 	uint16_t address = (uint8_t)(bus->getValue(pc) | (bus->getValue(++pc) << 8) + x);
		return address;
	}

	if(mode == 17){ // Absolute Y
		uint16_t address = (uint8_t)(bus->getValue(pc) | (bus->getValue(++pc) << 8) + y);
		return address;
	}

	if(mode == 19){ // Index Indirect
		uint8_t temp = bus->getValue(pc);
		uint8_t temp2 = temp + x;
		uint8_t temp3 = bus->getValue(temp2) | (bus->getValue(temp2 + 1) << 8);
		return bus->getValue(temp3);
	}

	if(mode == 20){ // Indirect Index
		uint8_t temp = bus->getValue(pc);
		uint8_t temp2 = bus->getValue(temp);
		uint8_t temp3 = temp2 + y;
		return bus->getValue(temp3);
	}

	return 0xFFFF;
}

void CPU6502::executeTest(uint8_t opcode){
	cout <<  "opcode: " << hex << (int)opcode << endl;

	if(opcode == 0xA9){
		uint16_t temp = handleMode(Immediate, pc);
		lda(temp);
	}

	if(opcode == 0xA5){
		uint16_t temp = handleMode(ZeroPage, pc);
		lda(temp);
	}

	if(opcode == 0xB5){
		uint16_t temp = handleMode(ZeroPageX, pc);
		lda(temp);
	}

	if(opcode == 0xAD){
		uint16_t temp = handleMode(Absolute, pc);
		lda(temp);
	}

	if(opcode == 0xBD){
		uint16_t temp = handleMode(AbsoluteX, pc);
		lda(temp);
	}

	if(opcode == 0xB9){
		uint16_t temp = handleMode(AbsoluteY, pc);
		lda(temp);
	}

	if(opcode == 0xA1){
		uint16_t temp = handleMode(IndexedIndirect, pc);
		lda(temp);
	}

	if(opcode == 0xB1){
		uint16_t temp = handleMode(IndirectIndexed, pc);
		lda(temp);
	}

	if(opcode == 0xA2){
		uint16_t temp = handleMode(Implicit);

	}

	if(opcode == 0x00){
		handleMode(Implicit, pc);
		brk();
	}

	if(opcode == 0x78){
		sei();
	}

	if(opcode == 0xD8){
		cld();
	}

	if(opcode == 0xA9){
		pc++;
		lda(pc);
	}

	if(opcode == 0x8D){
		pc++;
		uint16_t address = bus->getValue(pc);
		pc++;
		address |= bus->getValue(pc) << 8;
	}

	pc++;
}
