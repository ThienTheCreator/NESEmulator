#include <iostream>
#include <cstdint>

#include "bus.h"
#include "cpu6502.h"

using namespace std;

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
void CPU6502::lda(uint8_t value){
	a = value;
	
	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Load X Register
void CPU6502::ldx(uint8_t value){
	x = value;

	bool z = x == 0;
	handleFlag(Zero, z);

	bool n = (x & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Load Y Register
void CPU6502::ldy(uint16_t value){
	y = value;

	bool z = y == 0;
	handleFlag(Zero, z);

	bool n = (y & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Store Accumulator
void CPU6502::sta(uint16_t address){
	bus->cpuWrite(address, a);
}

// Store X Register
void CPU6502::stx(uint16_t address){
	bus->cpuWrite(address, x);
}

// Store Y Register
void CPU6502::sty(uint16_t address){
	bus->cpuWrite(address, y);
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
	a = y;

	bool z = a == 0;
	handleFlag(Zero, z);
	
	bool n = (a & (1 << 7)) != 0; 
	handleFlag(Negative, n);
}

/*
** Stack Operation
*/

// Transfer Stack Pointer to X
void CPU6502::tsx(){
	x = s;

	bool z = x == 0;
	handleFlag(Zero, z);

	bool n = x & (1 << 7);
	handleFlag(Negative, n);
}

// Transfer X to Stack Pointer
void CPU6502::txs(){
	s = x;
}

// Push Accumulator
void CPU6502::pha(){
	bus->cpuWrite(s | 0x100, a);
	s--;
}

// Push Processor Status
void CPU6502::php(){
	bus->cpuWrite(s | 0x100, p | 0x10);
	s--;
}

// Pull Accumulator
void CPU6502::pla(){
	s++;
	a = bus->cpuRead(s | 0x100);

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Pull Processor Status
void CPU6502::plp(){
	s++;
	p = bus->cpuRead(s | 0x100) & 0xEF | 0x20;
}

/*
** Logical 
*/

// Logical and
void CPU6502::andL(uint8_t value){
	a = a & value;

	bool z = a == 0;
	handleFlag(Zero, z);
	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Exclusive OR
void CPU6502::eor(uint8_t value){
	a = a ^ value;

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
	
	bool z = a == 0;
	handleFlag(Zero, z);
}

// Logical Inclusive OR
void CPU6502::ora(uint8_t value){
	a = a | value;

	bool c = a == 0;
	handleFlag(Carry, a == 0);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// bit test
void CPU6502::bit(uint8_t value){
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
void CPU6502::adc(uint8_t value){
	bool a7 = (a & (1 << 7)) != 0;
	bool m7 = (value & (1 << 7)) != 0;

	bool carryIn = getFlag(Carry);

	bool carryOut = a + value + carryIn > 0xFF;
	a = a + value + carryIn;
	handleFlag(Carry, carryOut);

	handleFlag(Zero, a == 0);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool signResult = (a & (1 << 7)) != 0;
	bool v = (a7 == m7) && (a7 ^ signResult);
	handleFlag(Overflow, v);
}

// Subtract with Carry
void CPU6502::sbc(uint8_t value){
	bool a7 = (a & (1 << 7)) != 0;
	bool m7 = (value & (1 << 7)) != 0;

	bool carryIn = getFlag(Carry);

	bool carryOut = a < value - 1 + carryIn;
	a = a + ~(value) + carryIn;

	handleFlag(Carry, !carryOut);

	handleFlag(Zero, a == 0);

	bool n = (a & (1 << 7)) != 0;
	handleFlag(Negative, n);

	bool signResult = (a & (1 << 7)) != 0;
	bool v = (a7 ^ m7) && (a7 ^ signResult);
	handleFlag(Overflow, v);
}

// Compare
void CPU6502::cmp(uint8_t value){
	uint8_t cmpValue = a - value;

	bool c = a >= value;
	handleFlag(Carry, c);

	bool z = a == value;
	handleFlag(Zero, z);

	bool n = (cmpValue & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Compare X Register
void CPU6502::cpx(uint8_t value){
	uint8_t result = x - value;

	bool c = x >= value;
	handleFlag(Carry, c);

	bool z = x == value;
	handleFlag(Zero, z);

	bool n = (result & (1 << 7)) != 0;
	handleFlag(Negative, n);
}

// Compare Y Register
void CPU6502::cpy(uint8_t value){
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
	uint8_t value = bus->cpuRead(address);
	value = value + 1;

	bus->cpuWrite(address, value);

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
	uint8_t value = bus->cpuRead(address);
	value = value - 1;

	bus->cpuWrite(address, value);

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
void CPU6502::asl(uint16_t address, bool isAccumulatorMode){
	uint8_t temp;
	if(isAccumulatorMode){
		temp = a;
	}else{
		temp = bus->cpuRead(address);
	}

	uint8_t c = temp & (1 << 7);
	handleFlag(Carry, c);

	temp = temp << 1;

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (temp & (1 << 7)) != 0;
	handleFlag(Negative, n);

	if(isAccumulatorMode){
		a = temp;	
	}else{
		bus->cpuWrite(address, temp);
	}
}

// Logical Shift Right
void CPU6502::lsr(uint16_t address, bool isAccumulatorMode){
	uint8_t temp;
	if(isAccumulatorMode){
		temp = a;
	}else{
		temp = bus->cpuRead(address);
	}
	
	bool c = temp & 1;
	handleFlag(Carry, c);
	
	temp = temp >> 1;

	bool z = temp == 0;
	handleFlag(Zero, z);

	bool v = temp & (1 << 7);
	handleFlag(Overflow, v);

	if(isAccumulatorMode){ 
		a = temp;
	}else{
		bus->cpuWrite(address, temp);
	}
}

// Rotate Left
void CPU6502::rol(uint16_t address, bool isAccumulatorMode){
	uint8_t temp;
	if(isAccumulatorMode){
		temp = a;
	}else{
		temp = bus->cpuRead(address);
	}

	bool bitSeven = temp & (1 << 7);

	temp = temp << 1;

	bool c = getFlag(Carry);
	temp = temp | c;

	handleFlag(Carry, bitSeven);

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (temp & (1 << 7)) != 0;
	handleFlag(Negative, n);

	if(isAccumulatorMode){ 
		a = temp;
	}else{
		bus->cpuWrite(address, temp);
	}
}

// Rotate Right
void CPU6502::ror(uint16_t address, bool isAccumulatorMode){
	uint8_t temp;
	if(isAccumulatorMode){
		temp = a;
	}else{
		temp = bus->cpuRead(address);
	}

	bool bitZero = temp & 1;

	temp = temp >> 1;

	bool c = getFlag(Carry);
	temp = temp | (c << 7);

	handleFlag(Carry, bitZero);

	bool z = a == 0;
	handleFlag(Zero, z);

	bool n = (temp & (1 << 7)) != 0;
	handleFlag(Negative, n);

	if(isAccumulatorMode){ 
		a = temp;
	}else{
		bus->cpuWrite(address, temp);
	}
}

/*
** Jumps and Calls
*/

// Jump
void CPU6502::jmp(uint16_t address){
	pc = address - 1;
}

// Jump to Subroutine
void CPU6502::jsr(uint16_t address){
	bus->cpuWrite(s | 0x100, pc >> 8);
	s--;
	bus->cpuWrite(s | 0x100, pc);
	s--;
	pc = address - 1;
}

// Return from Subroutine
void CPU6502::rts(){
	s++;
	pc = bus->cpuRead(s | 0x100);
	s++;
	pc |= bus->cpuRead(s | 0x100) << 8;
}

/*
** Branches
*/

// Branch if Carry Clear
void CPU6502::bcc(int8_t displacement){ // TODO
	bool carryFlag = getFlag(Carry);
	
	if(!carryFlag){
		pc += displacement;
	}
}

// Branch if Carry Set
void CPU6502::bcs(int8_t displacement){
	bool carryFlag = getFlag(Carry);
	
	if(carryFlag){
		pc += displacement;	
	}
}

// Branch if Equal
void CPU6502::beq(int8_t displacement){ // TODO
	bool zeroFlag = getFlag(Zero);
	
	if(zeroFlag){
		pc += displacement;
	}
}


// Branch if Minus
void CPU6502::bmi(int8_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(negativeFlag){
		pc += displacement;
	}
}

// Branch if Not Equal
void CPU6502::bne(int8_t displacement){
	bool zeroFlag = getFlag(Zero);
	
	if(!zeroFlag){
		pc += displacement;
	}
}

// Branch if Positive
void CPU6502::bpl(int8_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(!negativeFlag){
		pc += displacement;
	}
}

// Branch if Overflow Clear
void CPU6502::bvc(int8_t displacement){
	bool overflowFlag = getFlag(Overflow);
	
	if(!overflowFlag){
		pc += displacement;
	}
}

// Branch if Overflow Set
void CPU6502::bvs(int8_t displacement){
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
	waitCycle = 2;
}

/*
** System Functions
*/

// Force an Interrupt
void CPU6502::brk(){
	bus->cpuWrite(s | 0x100, pc);
	s--;
	bus->cpuWrite(s | 0x100, pc >> 8);
	s--;
	bus->cpuWrite(s | 0x100, p);
	s--;

	uint16_t temp = bus->cpuRead(0xFFFE);
	temp = temp | bus->cpuRead(0xFFFF) << 8;

	pc = temp;

	handleFlag(Break, true);
}

// No Operation
void CPU6502::nop(){

}

// Return from Interrupt
void CPU6502::rti(){
	s++;
	p = bus->cpuRead(s | 0x100);
	s++;
	pc = bus->cpuRead(s | 0x100);
	s++;
	pc |= bus->cpuRead(s | 0x100) << 8;
	
	pc--;
}

void CPU6502::irq(){
	setFlag(Interrupt);
}

void CPU6502::nmi(){
	bus->cpuWrite(s | 0x100, pc >> 8);
	s--;
	bus->cpuWrite(s | 0x100, pc);
	s--;
	
	handleFlag(Interrupt, 1);
	handleFlag(Break, 0);
	
	bus->cpuWrite(s | 0x100, p);
	s--;

	uint16_t temp = bus->cpuRead(0xFFFA);
	temp = temp | bus->cpuRead(0xFFFB) << 8;

	pc = temp;
}

void CPU6502::reset(){
	setFlag(Interrupt);
}


uint16_t CPU6502::getModeInstruction(int mode){
	pc++;

	if(mode == Immediate){ // Immediate
		return bus->cpuRead(pc);
		waitCycle = 2;
	}

	if(mode == ZeroPage){ // Zero Page
		return bus->cpuRead(pc);

	}

	if(mode == ZeroPageX){ // Zero Page X
		uint8_t address = bus->cpuRead(pc);
		return (uint8_t)(address + x);
	}

	if(mode == ZeroPageY){
		uint8_t address = bus->cpuRead(pc);
		return (uint8_t)(address + y);
		waitCycle = 4;
	}

	if(mode == Relative){
		uint8_t displacement = bus->cpuRead(pc);

		// TODO: add +1 branch, add +2 new page
		waitCycle = 2;

		return displacement;
	}

	if(mode == Absolute){ // Absolute
		uint16_t address = bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		return address;
	}

	if(mode == AbsoluteX){ // Absolute X
	 	uint16_t address = (uint8_t)(bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8) + x);
		return address;
	}

	if(mode == AbsoluteY){ // Absolute Y
		uint16_t address = (uint8_t)(bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8) + y);
		return address;
	}

	if(mode == IndexedIndirect){ // Index Indirect
		uint8_t temp = bus->cpuRead(pc);
		uint8_t temp2 = temp + x;
		uint8_t temp3 = bus->cpuRead(temp2) | (bus->cpuRead(temp2 + 1) << 8);
		waitCycle = 6;
		return bus->cpuRead(temp3);
	}

	if(mode == IndirectIndexed){ // Indirect Index
		uint8_t temp = bus->cpuRead(pc);
		uint8_t temp2 = bus->cpuRead(temp);
		uint8_t temp3 = temp2 + y;
		return bus->cpuRead(temp3);
	}

	return 0;
}

void CPU6502::executeInstruction(uint8_t opcode){
	if(opcode == 0xA9){
		uint16_t value = getModeInstruction(Immediate);
		lda(value);
	}

	if(opcode == 0xA5){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle = 3;
	}

	if(opcode == 0xB5){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle = 4;
	}

	if(opcode == 0xAD){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle = 4;
	}

	if(opcode == 0xBD){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		lda(value);
	}

	if(opcode == 0xB9){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		lda(value);
	}

	if(opcode == 0xA1){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		lda(value);
	}

	if(opcode == 0xB1){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		lda(value);
	}

	if(opcode == 0xA2){
		uint16_t value = getModeInstruction(Immediate);
		ldx(value);
	}

	if(opcode == 0xA6){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		ldx(value);
		waitCycle = 3;
	}

	if(opcode == 0xB6){
		uint16_t address = getModeInstruction(ZeroPageY);
		uint8_t value = bus->cpuRead(address);
		ldx(value);
	}

	if(opcode == 0xAE){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		ldx(value);
		waitCycle = 4;
	}

	if(opcode == 0xBE){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint16_t value = bus->cpuRead(address);
		ldx(value);
	}

	if(opcode == 0xA0){
		uint16_t value = getModeInstruction(Immediate);
		ldy(value);
	}

	if(opcode == 0xA4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
		waitCycle = 3;
	}

	if(opcode == 0xB4){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
		waitCycle = 4;
	}

	if(opcode == 0xAC){
		uint16_t address = getModeInstruction(Absolute);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
		waitCycle = 4;
	}

	if(opcode == 0xBC){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
	}

	if(opcode == 0x85){
		uint16_t address = getModeInstruction(ZeroPage);
		sta(address);
		waitCycle = 3;
	}

	if(opcode == 0x95){
		uint16_t address = getModeInstruction(ZeroPageX);
		sta(address);
		waitCycle = 4;
	}

	if(opcode == 0x8D){
		uint16_t address = getModeInstruction(Absolute);
		sta(address);
		waitCycle = 4;
	}

	if(opcode == 0x9D){
		uint16_t address = getModeInstruction(AbsoluteX);
		sta(address);
	}

	if(opcode == 0x99){
		uint16_t address = getModeInstruction(AbsoluteY);
		sta(address);
	}

	if(opcode == 0x81){
		uint16_t address = getModeInstruction(IndirectIndexed);
		sta(address);
	}

	if(opcode == 0x91){
		uint16_t address = getModeInstruction(IndexedIndirect);
		sta(address);
	}

	if(opcode == 0x86){
		uint16_t address = getModeInstruction(ZeroPage);
		stx(address);
		waitCycle = 3;
	}

	if(opcode == 0x96){
		uint16_t address = getModeInstruction(ZeroPageY);
		stx(address);
	}

	if(opcode == 0x8E){
		uint16_t address = getModeInstruction(Absolute);
		stx(address);
		waitCycle = 4;
	}

	if(opcode == 0x84){
		uint16_t address = getModeInstruction(ZeroPage);
		sty(address);
		waitCycle = 3;
	}

	if(opcode == 0x94){
		uint16_t address = getModeInstruction(ZeroPageX);
		sty(address);
		waitCycle = 4;
	}

	if(opcode == 0x8C){
		uint16_t address = getModeInstruction(Absolute);
		sty(address);
		waitCycle = 4;
	}

	if(opcode == 0xAA){
		tax();
	}

	if(opcode == 0xA8){
		tay();
	}

	if(opcode == 0x8A){
		txa();
	}

	if(opcode == 0x98){
		tya();
	}

	if(opcode == 0xBA){
		tsx();
	}

	if(opcode == 0x9A){
		txs();
	}

	if(opcode == 0x48){
		pha();
	}

	if(opcode == 0x08){
		php();
	}

	if(opcode == 0x68){
		pla();
	}

	if(opcode == 0x28){
		plp();
	}

	if(opcode == 0x29){
		uint8_t value = getModeInstruction(Immediate);
		andL(value);
	}

	if(opcode == 0x25){
		uint8_t address = getModeInstruction(ZeroPage);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle = 3;
	}

	if(opcode == 0x35){
		uint8_t address = getModeInstruction(ZeroPageX);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle = 4;
	}

	if(opcode == 0x2D){
		uint8_t address = getModeInstruction(Absolute);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle = 4;
	}

	if(opcode == 0x3D){
		uint8_t address = getModeInstruction(AbsoluteX);
		int8_t value = bus->cpuRead(address);
		andL(value);
	}

	if(opcode == 0x39){
		uint8_t address = getModeInstruction(AbsoluteY);
		int8_t value = bus->cpuRead(address);
		andL(value);
	}

	if(opcode == 0x21){
		uint8_t address = getModeInstruction(IndexedIndirect);
		int8_t value = bus->cpuRead(address);
		andL(value);
	}

	if(opcode == 0x31){
		uint8_t address = getModeInstruction(IndirectIndexed);
		int8_t value = bus->cpuRead(address);
		andL(value);
	}

	if(opcode == 0x49){
		uint8_t value = getModeInstruction(Immediate);
		eor(value);
	}

	if(opcode == 0x45){
		uint8_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle = 3;
	}

	if(opcode == 0x55){
		uint8_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle = 4;
	}

	if(opcode == 0x4D){
		uint8_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle = 4;
	}

	if(opcode == 0x5D){
		uint8_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	}

	if(opcode == 0x59){
		uint8_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	}

	if(opcode == 0x41){
		uint8_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	}

	if(opcode == 0x51){
		uint8_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	}

	if(opcode == 0x09){
		uint8_t value = getModeInstruction(Immediate);
		ora(value);
	}

	if(opcode == 0x05){
		uint8_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle = 3;
	}

	if(opcode == 0x15){
		uint8_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle = 4;
	}

	if(opcode == 0x0D){
		uint8_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle = 4;
	}

	if(opcode == 0x1D){
		uint8_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		ora(value);
	}

	if(opcode == 0x19){
		uint8_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		ora(value);
	}

	if(opcode == 0x01){
		uint8_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		ora(value);
	}

	if(opcode == 0x11){
		uint8_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		ora(value);
	}

	if(opcode == 0x24){
		uint8_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		bit(value);
		waitCycle = 3;
	}

	if(opcode == 0x2C){
		uint8_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		bit(value);
		waitCycle = 4;
	}

	if(opcode == 0x69){
		uint8_t value = getModeInstruction(Immediate);
		adc(value);
	}

	if(opcode == 0x65){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle = 3;
	}

	if(opcode == 0x75){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle = 4;
	}

	if(opcode == 0x6D){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle = 4;
	}

	if(opcode == 0x7D){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		adc(value);
	}

	if(opcode == 0x79){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		adc(value);
	}

	if(opcode == 0x61){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		adc(value);
	}

	if(opcode == 0x71){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		adc(value);
	}

	if(opcode == 0xE9){
		uint8_t value = getModeInstruction(Immediate);
		sbc(value);
	}

	if(opcode == 0xE5){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle = 3;
	}

	if(opcode == 0xF5){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle = 4;
	}

	if(opcode == 0xED){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle = 4;
	}

	if(opcode == 0xFD){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
	}

	if(opcode == 0xF9){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
	}

	if(opcode == 0xE1){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
	}

	if(opcode == 0xF1){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
	}

	if(opcode == 0xC9){
		uint8_t value = getModeInstruction(Immediate);
		cmp(value);
	}

	if(opcode == 0xC5){
		uint8_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle = 3;
	}

	if(opcode == 0xD5){
		uint8_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle = 4;
	}

	if(opcode == 0xCD){
		uint8_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle = 4;
	}
	
	if(opcode == 0xDD){
		uint8_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
	}
	
	if(opcode == 0xD9){
		uint8_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
	}
	
	if(opcode == 0xC1){
		uint8_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
	}
	
	if(opcode == 0xD1){
		uint8_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
	}

	if(opcode == 0xE0){
		uint8_t value = getModeInstruction(Immediate);
		cpx(value);
	}

	if(opcode == 0xE4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cpx(value);
		waitCycle = 3;
	}

	if(opcode == 0xEC){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cpx(value);
		waitCycle = 4;
	}

	if(opcode == 0xC0){
		uint8_t value = getModeInstruction(Immediate);
		cpy(value);
	}

	if(opcode == 0xC4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cpy(value);
		waitCycle = 3;
	}

	if(opcode == 0xCC){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cpy(value);
		waitCycle = 4;
	}

	if(opcode == 0xE6){
		uint16_t address = getModeInstruction(ZeroPage);
		inc(address);
		waitCycle = 5;
	}

	if(opcode == 0xF6){
		uint16_t address = getModeInstruction(ZeroPageX);
		inc(address);
		waitCycle = 6;
	}

	if(opcode == 0xEE){
		uint16_t address = getModeInstruction(Absolute);
		inc(address);
		waitCycle = 6;
	}

	if(opcode == 0xFE){
		uint16_t address = getModeInstruction(AbsoluteX);
		inc(address);
	}

	if(opcode == 0xE8){
		inx();
	}

	if(opcode == 0xC8){
		iny();
	}

	if(opcode == 0xC6){
		uint16_t address = getModeInstruction(ZeroPage);
		dec(address);
		waitCycle = 5;
	}

	if(opcode == 0xD6){
		uint16_t address = getModeInstruction(ZeroPageX);
		dec(address);
		waitCycle = 6;
	}

	if(opcode == 0xCE){
		uint16_t address = getModeInstruction(Absolute);
		dec(address);
		waitCycle = 6;
	}

	if(opcode == 0xDE){
		uint16_t address = getModeInstruction(AbsoluteX);
		dec(address);
	}

	if(opcode == 0xCA){
		dex();
	}

	if(opcode == 0x88){
		dey();
	}

	if(opcode == 0x0A){
		asl(a, true);
	}

	if(opcode == 0x06){
		uint16_t address = getModeInstruction(ZeroPage);
		asl(address, false);
		waitCycle = 5;
	}

	if(opcode == 0x16){
		uint16_t address = getModeInstruction(ZeroPageX);
		asl(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x0E){
		uint16_t address = getModeInstruction(Absolute);
		asl(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x1E){
		uint16_t address = getModeInstruction(AbsoluteX);
		asl(address, false);
	}

	if(opcode == 0x4A){
		lsr(a, true);
	}

	if(opcode == 0x46){
		uint16_t address = getModeInstruction(ZeroPage);
		lsr(address, false);
		waitCycle = 5;
	}

	if(opcode == 0x56){
		uint16_t address = getModeInstruction(ZeroPageX);
		lsr(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x4E){
		uint16_t address = getModeInstruction(Absolute);
		lsr(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x5E){
		uint16_t address = getModeInstruction(AbsoluteX);
		lsr(address, false);
	}

	if(opcode == 0x2A){
		rol(a, true);
	}

	if(opcode == 0x26){
		uint16_t address = getModeInstruction(ZeroPage);
		rol(address, false);
		waitCycle = 5;
	}

	if(opcode == 0x36){
		uint16_t address = getModeInstruction(ZeroPageX);
		rol(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x2E){
		uint16_t address = getModeInstruction(Absolute);
		rol(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x3E){
		uint16_t address = getModeInstruction(AbsoluteX);
		rol(address, false);
	}

	if(opcode == 0x6A){
		ror(a, true);
	}

	if(opcode == 0x66){
		uint16_t address = getModeInstruction(ZeroPage);
		ror(address, false);
		waitCycle = 5;
	}

	if(opcode == 0x76){
		uint16_t address = getModeInstruction(ZeroPageX);
		ror(address, false);
		waitCycle = 6;
	}

	if(opcode == 0x6E){
		uint16_t address = getModeInstruction(Absolute);
		ror(address, false);
	}

	if(opcode == 0x7E){
		uint16_t address = getModeInstruction(AbsoluteX);
		ror(address, false);
	}

	if(opcode == 0x4C){
		uint16_t address = getModeInstruction(Absolute);
		jmp(address);
		waitCycle = 3;
	}

	if(opcode == 0x6C){
		uint16_t address = getModeInstruction(Indirect);
		jmp(address);
	}

	if(opcode == 0x20){
		uint16_t address = getModeInstruction(Absolute);
		jsr(address);
		waitCycle = 6;
	}

	if(opcode == 0x60){
		rts();
	}

	if(opcode == 0x90){
		uint8_t displacement = getModeInstruction(Relative);
		bcc(displacement);
	}

	if(opcode == 0xB0){
		uint8_t displacement = getModeInstruction(Relative);
		bcs(displacement);
	}

	if(opcode == 0xF0){
		uint8_t displacement = getModeInstruction(Relative);
		beq(displacement);
	}

	if(opcode == 0x30){
		uint16_t displacement = getModeInstruction(Relative);
		bmi(displacement);
	}

	if(opcode == 0xD0){
		uint16_t displacement = getModeInstruction(Relative);
		bne(displacement);
	}

	if(opcode == 0x10){
		uint16_t displacement = getModeInstruction(Relative);
		bpl(displacement);
	}

	if(opcode == 0x50){
		uint16_t displacement = getModeInstruction(Relative);
		bvc(displacement);
	}

	if(opcode == 0x70){
		uint16_t displacement = getModeInstruction(Relative);
		bvs(displacement);
	}

	if(opcode == 0x18){
		clc();
	}

	if(opcode == 0xD8){
		cld();
	}

	if(opcode == 0x58){
		cli();
	}

	if(opcode == 0xB8){
		clv();
	}

	if(opcode == 0x38){
		sec();
	}

	if(opcode == 0xF8){
		sed();
	}

	if(opcode == 0x78){
		sei();
	}

	if(opcode == 0x00){
		brk();
	}

	if(opcode == 0xEA){
		nop();
	}

	if(opcode == 0x40){
		rti();
	}

	pc++;
}
