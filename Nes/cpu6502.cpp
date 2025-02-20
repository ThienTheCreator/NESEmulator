#include <iomanip> 

#include "bus.h"
#include "cpu6502.h"

using namespace std;


CPU6502::CPU6502(){
}

void CPU6502::reset(){
	pc = bus->cpuRead(0xFFFC) | (bus->cpuRead(0xFFFD) << 8);

	a = 0;
	x = 0;
	y = 0;
	s = 0xFD;
	p = 0x24;

	waitCycle = 8;
}

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

uint16_t CPU6502::getModeInstruction(int mode){
	pc++;

	if(mode == Immediate){ // Immediate
		return bus->cpuRead(pc);
	}

	if(mode == ZeroPage){ // Zero Page
		return bus->cpuRead(pc) & 0xFF;
	}

	if(mode == ZeroPageX){ // Zero Page X
		uint16_t address = bus->cpuRead(pc);
		return (uint8_t)(address + x) & 0xFF;
	}

	if(mode == ZeroPageY){
		uint16_t address = bus->cpuRead(pc);
		return (uint8_t)(address + y) & 0xFF;
	}

	if(mode == Relative){
		uint8_t displacement = bus->cpuRead(pc);

		return displacement;
	}

	if(mode == Absolute){ // Absolute
		uint16_t address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		
		return address;
	}

	if(mode == AbsoluteX){ // Absolute X
	 	uint16_t address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		uint16_t temp = address;
		address += x;
		
		if((address & 0xFF00) != (temp & 0xFF00)){
			pageCrossed = true;
		} else {
			pageCrossed = false;
		}
		
		return address;
	}

	if(mode == AbsoluteY){ // Absolute Y
		uint16_t address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		uint16_t temp = address;
		address += y;

		if((address & 0xFF00) != (temp & 0xFF00)){
			pageCrossed = true;
		} else {
			pageCrossed = false;
		}
		
		return address;
	}

	if(mode == Indirect){
		uint16_t address = bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		address = bus->cpuRead(address) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);

		return address;
	}

	if(mode == IndexedIndirect){ // Index Indirect: (indirect,x)
		uint16_t address = bus->cpuRead(pc);
		address = (address + x) & 0xFF;
		address = bus->cpuRead(address & 0xFF) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);
		
		return address;
	}

	if(mode == IndirectIndexed){ // Indirect Index: (indirect),y
		uint16_t address = bus->cpuRead(pc);
		address = bus->cpuRead(address) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);
		
		uint16_t temp = address;
		address = address + y;
		
		if((address & 0xFF00) != (temp & 0xFF00)){
			pageCrossed = true;
		} else {
			pageCrossed = false;
		}
		
		return address;
	}

	return 0;
}

/* 
** Load or Store Operations 
*/

// Load Accumulator
void CPU6502::lda(uint8_t value){
	a = value;
	handleFlag(Zero, a == 0);
	handleFlag(Negative, a & 0x80);
}

// Load X Register
void CPU6502::ldx(uint8_t value){
	x = value;
	handleFlag(Zero, x == 0);
	handleFlag(Negative, x & 0x80);
}

// Load Y Register
void CPU6502::ldy(uint16_t value){
	y = value;
	handleFlag(Zero, y == 0);
	handleFlag(Negative, y & 0x80);
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

	handleFlag(Zero, x == 0);

	handleFlag(Negative, x & 0x80);
}

// Transfer Accumulator to Y
void CPU6502::tay(){
	y = a;

	handleFlag(Zero, y == 0);

	handleFlag(Negative, y & 0x80);
}

// Transfer X to Accumulator
void CPU6502::txa(){
	a = x;

	handleFlag(Zero, a == 0);

	handleFlag(Negative, a & 0x80);
}

// Transfer Y to Accumulator
void CPU6502::tya(){
	a = y;

	handleFlag(Zero, a == 0);
	
	handleFlag(Negative, a & 0x80);
}

/*
** Stack Operation
*/

// Transfer Stack Pointer to X
void CPU6502::tsx(){
	x = s;

	handleFlag(Zero, x == 0);

	handleFlag(Negative, x & 0x80);
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
	handleFlag(Break, true);
	handleFlag(Unused, true);
	bus->cpuWrite(0x100 | s, p);
	handleFlag(Break, false);
	handleFlag(Unused, false);
	s--;
}

// Pull Accumulator
void CPU6502::pla(){
	s++;
	a = bus->cpuRead(0x100 | s);

	handleFlag(Zero, a == 0);

	handleFlag(Negative, a & 0x80);
}

// Pull Processor Status
void CPU6502::plp(){
	s++;
	p = bus->cpuRead(0x100 | s);
	handleFlag(Unused, 1);
}

/*
** Logical 
*/

// Logical and
void CPU6502::andL(uint8_t value){
	a = a & value;

	handleFlag(Zero, a == 0);
	handleFlag(Negative, a & 0x80);
}

// Exclusive OR
void CPU6502::eor(uint8_t value){
	a = a ^ value;
	handleFlag(Negative, a & 0x80);
	handleFlag(Zero, a == 0);
}

// Logical Inclusive OR
void CPU6502::ora(uint8_t value){
	a = a | value;
	handleFlag(Zero, a == 0);
	handleFlag(Negative, a & 0x80);
}

// bit test
void CPU6502::bit(uint8_t value){
	handleFlag(Negative, value & 0x80);

	handleFlag(Overflow, value & 0x40);

	handleFlag(Zero, (a & value) == 0);
}

/*
** Arithmetic
*/

// ADC - Add with Carry
void CPU6502::adc(uint8_t value){
	bool a7 = (a & (1 << 7)) != 0;
	bool m7 = (value & (1 << 7)) != 0;

	bool carryIn = getFlag(Carry);

	bool carryOut = (a + value + carryIn) > 0xFF;
	a = a + value + carryIn;
	handleFlag(Carry, carryOut);

	handleFlag(Zero, a == 0);

	handleFlag(Negative, a & 0x80);

	bool signResult = (a & 0x80) != 0;
	bool v = (a7 == m7) && (a7 ^ signResult);
	handleFlag(Overflow, v);
}

// Subtract with Carry
void CPU6502::sbc(uint8_t value){
	bool a7 = (a & (1 << 7)) != 0;
	bool m7 = (value & (1 << 7)) != 0;

	bool carryIn = getFlag(Carry);

	bool carryOut = a < value - 1 + carryIn;
	a = a - value - !carryIn;

	handleFlag(Carry, !carryOut);

	handleFlag(Zero, a == 0);

	handleFlag(Negative, a & 0x80);

	bool signResult = (a & 0x80) != 0;
	bool v = (a7 ^ m7) && (a7 ^ signResult);
	handleFlag(Overflow, v);
}

// Compare
void CPU6502::cmp(uint8_t value){
	uint8_t cmpValue = a - value;

	handleFlag(Carry, a >= value);

	handleFlag(Zero, a == value);

	handleFlag(Negative, cmpValue & 0x80);
}

// Compare X Register
void CPU6502::cpx(uint8_t value){
	uint8_t cmpValue = x - value;

	handleFlag(Carry, x >= value);

	handleFlag(Zero, x == value);

	handleFlag(Negative, cmpValue & 0x80);
}

// Compare Y Register
void CPU6502::cpy(uint8_t value){
	uint8_t cmpValue = y - value;

	handleFlag(Carry, y >= value);

	handleFlag(Zero, y == value);

	handleFlag(Negative, cmpValue & 0x80);
}

/*
** Increment and Decrements
*/

// Increment Memory
void CPU6502::inc(uint16_t address){
	uint8_t value = bus->cpuRead(address);
	value = value + 1;

	bus->cpuWrite(address, value);

	handleFlag(Negative, value & 0x80);

	handleFlag(Zero, value == 0);
}

// Increment X Register
void CPU6502::inx(){
	x++;

	handleFlag(Negative, x & 0x80);

	handleFlag(Zero, x == 0);

}

// Increment Y Register
void CPU6502::iny(){
	y++;

	handleFlag(Negative, y & 0x80);

	handleFlag(Zero, y == 0);
}

// Decrement Memory
void CPU6502::dec(uint16_t address){
	uint8_t value = bus->cpuRead(address);
	value = value - 1;

	bus->cpuWrite(address, value);

	handleFlag(Negative, value & 0x80);

	handleFlag(Zero, value == 0);
}

// Decrement X Register
void CPU6502::dex(){
	x--;

	handleFlag(Negative, x & 0x80);

	handleFlag(Zero, x == 0);
}

// Decrement Y Register
void CPU6502::dey(){
	y--;

	handleFlag(Negative, y & 0x80);

	handleFlag(Zero, y == 0);
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

	uint8_t c = temp & 0x80;
	handleFlag(Carry, c);

	temp <<= 1;

	handleFlag(Zero, temp == 0);
	handleFlag(Negative, temp & 0x80);

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
	
	temp >>= 1;

	handleFlag(Zero, temp == 0);

	handleFlag(Negative, temp & 0x80);

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

	bool bitSeven = temp & 0x80;

	temp = temp << 1;

	bool c = getFlag(Carry);
	temp = temp | c;

	handleFlag(Carry, bitSeven);

	handleFlag(Zero, temp == 0);

	handleFlag(Negative, temp & 0x80);

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

	handleFlag(Zero, temp == 0);

	handleFlag(Negative, temp & 0x80);

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
	bus->cpuWrite(s | 0x100, (pc >> 8) & 0x00FF);
	s--;
	bus->cpuWrite(s | 0x100, pc & 0x00FF);
	s--;

	pc = address - 1;
}

// Return from Subroutine
void CPU6502::rts(){
	s++;
	pc = (uint16_t)bus->cpuRead(0x100 | s);
	s++;
	pc |= (uint16_t)bus->cpuRead(0x100 | s) << 8;
}

/*
** Branches
*/

// Branch if Carry Clear
void CPU6502::bcc(int8_t displacement){
	bool carryFlag = getFlag(Carry);
	
	if(!carryFlag){
		waitCycle++;
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;
	}
}

// Branch if Carry Set
void CPU6502::bcs(int8_t displacement){
	bool carryFlag = getFlag(Carry);
	
	if(carryFlag){
		waitCycle++;
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}

// Branch if Equal
void CPU6502::beq(int8_t displacement){ // TODO
	bool zeroFlag = getFlag(Zero);
	
	if(zeroFlag){
		waitCycle++;
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}


// Branch if Minus
void CPU6502::bmi(int8_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(negativeFlag){
		waitCycle++;
		
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}

// Branch if Not Equal
void CPU6502::bne(int8_t displacement){
	bool zeroFlag = getFlag(Zero);
	
	if(!zeroFlag){
		waitCycle++;
		
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}

// Branch if Positive
void CPU6502::bpl(int8_t displacement){
	bool negativeFlag = getFlag(Negative);
	
	if(!negativeFlag){
		waitCycle++;
		
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}

// Branch if Overflow Clear
void CPU6502::bvc(int8_t displacement){
	bool overflowFlag = getFlag(Overflow);
	
	if(!getFlag(Overflow)){
		waitCycle++;
		
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
	}
}

// Branch if Overflow Set
void CPU6502::bvs(int8_t displacement){
	bool overflowFlag = getFlag(Overflow);
	
	if(overflowFlag){
		waitCycle++;
		
		uint16_t address = pc + displacement;

		waitCycle++;
		if((pc & 0xFF00) != (address & 0xFF00))
			waitCycle++;

		pc = address;	
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
	pc++;

	handleFlag(Interrupt, true);
	bus->cpuWrite(s | 0x100, (pc >> 8) & 0x00FF);
	s--;
	bus->cpuWrite(s | 0x100, pc & 0x00FF);
	s--;
	
	handleFlag(Break, true);
	bus->cpuWrite(s | 0x100, p);
	s--;
	handleFlag(Break, false);
	
	pc = (uint16_t)bus->cpuRead(0xFFFE) | ((uint16_t)bus->cpuRead(0xFFFF) << 8);
}

// No Operation
void CPU6502::nop(){

}

// Return from Interrupt
void CPU6502::rti(){
	s++;
	p = bus->cpuRead(0x100 | s);
	p &= ~(1 << Break);
	p &= ~(1 << Unused);

	s++;
	pc = (uint16_t)bus->cpuRead(0x100 | s);
	s++;
	pc |= (uint16_t)bus->cpuRead(0x100 | s) << 8;
	
	pc--;
}

void CPU6502::lax(uint8_t value){
	lda(value);
	tax();
}

void CPU6502::irq(){
	if(getFlag(Interrupt) == 0){
		bus->cpuWrite(0x100 | s, (pc >> 8) & 0x00FF);
		s--;
		bus->cpuWrite(0x100 | s, pc & 0x00FF);
		s--;

		handleFlag(Break, false);
		handleFlag(Unused, true);
		handleFlag(Interrupt, true);
		bus->cpuWrite(0x100 | s, p);
		s--;

		uint16_t lo = bus->cpuRead(0xFFFE);
		uint16_t hi = bus->cpuRead(0xFFFF);
		pc = (hi << 8) | lo;

		waitCycle = 7;
	}
}

void CPU6502::nmi(){
	bus->cpuWrite(0x100 | s, (pc >> 8) & 0x00FF);
	s--;
	bus->cpuWrite(0x100 | s, pc & 0x00FF);
	s--;

	handleFlag(Break, 0);
	handleFlag(Unused, 1);
	handleFlag(Interrupt, 1);
	
	bus->cpuWrite(0x100 | s, p);
	s--;

	uint16_t temp = bus->cpuRead(0xFFFA);
	temp |= (bus->cpuRead(0xFFFB) << 8);

	pc = temp;

	waitCycle = 8;
}

void CPU6502::clock(){
	if(waitCycle <= 0){
		handleFlag(Unused, true);

		if(true){ // debug
			cout << uppercase << hex;
			cout << /* "PC:" << */ setw(4) << pc;
			cout << " A:" << setfill('0') << setw(2) << (int)a;
			cout <<	" X:" << setfill('0') << setw(2) << (int)x;
			cout << " Y:" << setfill('0') << setw(2) << (int)y; 
			cout << " S:" << setfill('0') << setw(2) << (int)s;
			cout << " P:";
			cout << (0b10000000 & p ? "N" : "n");
			cout << (0b01000000 & p ? "V" : "v");
			cout << "u";
			cout << (0b00010000 & p ? "B" : "b");
			cout << (0b00001000 & p ? "D" : "d");
			cout << (0b00000100 & p ? "I" : "i");
			cout << (0b00000010 & p ? "Z" : "z");
			cout << (0b00000001 & p ? "C" : "c");
			cout << endl;
		}
		executeInstruction(bus->cpuRead(pc));

		handleFlag(Unused, true);
	} 

	waitCycle--;
}

void CPU6502::executeInstruction(uint8_t opcode){
	if(opcode == 0xA9){
		uint8_t value = getModeInstruction(Immediate);
		lda(value);
		waitCycle += 2;
	}

	if(opcode == 0xA5){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle += 3;
	}

	if(opcode == 0xB5){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle += 4;
	}

	if(opcode == 0xAD){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		waitCycle += 4;
	}

	if(opcode == 0xBD){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xB9){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xA1){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		lda(value);

		waitCycle += 6;
	}

	if(opcode == 0xB1){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		lda(value);
		
		waitCycle += 5;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xA2){
		uint16_t value = getModeInstruction(Immediate);
		ldx(value);

		waitCycle += 2;
	}

	if(opcode == 0xA6){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		ldx(value);

		waitCycle += 3;
	}

	if(opcode == 0xB6){
		uint16_t address = getModeInstruction(ZeroPageY);
		uint8_t value = bus->cpuRead(address);
		ldx(value);

		waitCycle += 4;
	}

	if(opcode == 0xAE){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		ldx(value);

		waitCycle += 4;
	}

	if(opcode == 0xBE){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint16_t value = bus->cpuRead(address);
		ldx(value);
	
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xA0){
		uint16_t value = getModeInstruction(Immediate);
		ldy(value);

		waitCycle += 2;
	}

	if(opcode == 0xA4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint16_t value = bus->cpuRead(address);
		ldy(value);

		waitCycle += 3;
	}

	if(opcode == 0xB4){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
		
		waitCycle += 4;
	}

	if(opcode == 0xAC){
		uint16_t address = getModeInstruction(Absolute);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
		
		waitCycle += 4;
	}

	if(opcode == 0xBC){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint16_t value = bus->cpuRead(address);
		ldy(value);
	
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x85){
		uint16_t address = getModeInstruction(ZeroPage);
		sta(address);
		waitCycle += 3;
	}

	if(opcode == 0x95){
		uint16_t address = getModeInstruction(ZeroPageX);
		sta(address);
		waitCycle += 4;
	}

	if(opcode == 0x8D){
		uint16_t address = getModeInstruction(Absolute);
		sta(address);
		waitCycle += 4;
	}

	if(opcode == 0x9D){
		uint16_t address = getModeInstruction(AbsoluteX);
		sta(address);
		waitCycle += 5;
	}

	if(opcode == 0x99){
		uint16_t address = getModeInstruction(AbsoluteY);
		sta(address);
		waitCycle += 5;
	}

	if(opcode == 0x81){
		uint16_t address = getModeInstruction(IndexedIndirect);
		sta(address);
		waitCycle += 6;
	}

	if(opcode == 0x91){
		uint16_t address = getModeInstruction(IndirectIndexed);
		sta(address);
		waitCycle += 6;
	}

	if(opcode == 0x86){
		uint16_t address = getModeInstruction(ZeroPage);
		stx(address);
		waitCycle += 3;
	}

	if(opcode == 0x96){
		uint16_t address = getModeInstruction(ZeroPageY);
		stx(address);
		waitCycle += 4;
	}

	if(opcode == 0x8E){
		uint16_t address = getModeInstruction(Absolute);
		stx(address);
		waitCycle += 4;
	}

	if(opcode == 0x84){
		uint16_t address = getModeInstruction(ZeroPage);
		sty(address);
		waitCycle += 3;
	}

	if(opcode == 0x94){
		uint16_t address = getModeInstruction(ZeroPageX);
		sty(address);
		waitCycle += 4;
	}

	if(opcode == 0x8C){
		uint16_t address = getModeInstruction(Absolute);
		sty(address);
		waitCycle += 4;
	}

	if(opcode == 0xAA){
		tax();
		waitCycle += 2;
	}

	if(opcode == 0xA8){
		tay();
		waitCycle += 2;
	}

	if(opcode == 0x8A){
		txa();
		waitCycle += 2;
	}

	if(opcode == 0x98){
		tya();
		waitCycle += 2;
	}

	if(opcode == 0xBA){
		tsx();
		waitCycle += 2;
	}

	if(opcode == 0x9A){
		txs();
		waitCycle += 2;
	}

	if(opcode == 0x48){
		pha();
		waitCycle += 3;
	}

	if(opcode == 0x08){
		php();
		waitCycle += 3;
	}

	if(opcode == 0x68){
		pla();
		waitCycle += 4;
	}

	if(opcode == 0x28){
		plp();
		waitCycle += 4;
	}

	if(opcode == 0x29){
		uint8_t value = getModeInstruction(Immediate);
		andL(value);
		waitCycle += 2;
	}

	if(opcode == 0x25){
		uint16_t address = getModeInstruction(ZeroPage);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle += 3;
	}

	if(opcode == 0x35){
		uint16_t address = getModeInstruction(ZeroPageX);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle += 4;
	}

	if(opcode == 0x2D){
		uint16_t address = getModeInstruction(Absolute);
		int8_t value = bus->cpuRead(address);
		andL(value);
		waitCycle += 4;
	}

	if(opcode == 0x3D){
		uint16_t address = getModeInstruction(AbsoluteX);
		int8_t value = bus->cpuRead(address);
		andL(value);

		waitCycle += 4;

		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x39){
		uint16_t address = getModeInstruction(AbsoluteY);
		int8_t value = bus->cpuRead(address);
		andL(value);
	
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x21){
		uint16_t address = getModeInstruction(IndexedIndirect);
		int8_t value = bus->cpuRead(address);
		andL(value);

		waitCycle += 6;
	}

	if(opcode == 0x31){
		uint16_t address = getModeInstruction(IndirectIndexed);
		int8_t value = bus->cpuRead(address);
		andL(value);
	
		waitCycle += 5;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x49){
		uint8_t value = getModeInstruction(Immediate);
		eor(value);

		waitCycle += 2;
	}

	if(opcode == 0x45){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle += 3;
	}

	if(opcode == 0x55){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle += 4;
	}

	if(opcode == 0x4D){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		eor(value);
		waitCycle += 4;
	}

	if(opcode == 0x5D){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x59){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x41){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		eor(value);

		waitCycle += 6;
	}

	if(opcode == 0x51){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		eor(value);
	
		waitCycle += 5;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x09){
		uint8_t value = getModeInstruction(Immediate);
		ora(value);

		waitCycle += 2;
	}

	if(opcode == 0x05){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle += 3;
	}

	if(opcode == 0x15){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle += 4;
	}

	if(opcode == 0x0D){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle += 4;
	}

	if(opcode == 0x1D){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x19){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		
		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x01){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		ora(value);

		waitCycle += 6;
	}

	if(opcode == 0x11){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		ora(value);
		
		waitCycle += 5;
		
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x24){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		bit(value);
		waitCycle += 3;
	}

	if(opcode == 0x2C){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		bit(value);
		waitCycle += 4;
	}

	if(opcode == 0x69){
		uint8_t value = getModeInstruction(Immediate);
		adc(value);
		waitCycle += 2;
	}

	if(opcode == 0x65){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle += 3;
	}

	if(opcode == 0x75){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle += 4;
	}

	if(opcode == 0x6D){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle += 4;
	}

	if(opcode == 0x7D){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		
		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x79){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		adc(value);

		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0x61){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		adc(value);
		waitCycle += 6;
	}

	if(opcode == 0x71){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		adc(value);

		waitCycle += 5;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xE5){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle += 3;
	}

	if(opcode == 0xF5){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle += 4;
	}

	if(opcode == 0xED){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		waitCycle += 4;
	}

	if(opcode == 0xFD){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		
		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xF9){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
	
		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xE1){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		sbc(value);
		
		waitCycle += 6;
	}

	if(opcode == 0xF1){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		sbc(value);

		waitCycle += 5;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xC9){
		uint8_t value = getModeInstruction(Immediate);
		cmp(value);
		waitCycle += 2;
	}

	if(opcode == 0xC5){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle += 3;
	}

	if(opcode == 0xD5){
		uint16_t address = getModeInstruction(ZeroPageX);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle += 4;
	}

	if(opcode == 0xCD){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle += 4;
	}
	
	if(opcode == 0xDD){
		uint16_t address = getModeInstruction(AbsoluteX);
		uint8_t value = bus->cpuRead(address);
		cmp(value);

		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}
	
	if(opcode == 0xD9){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		cmp(value);

		waitCycle += 4;
		if(pageCrossed)
			waitCycle++;
	}
	
	if(opcode == 0xC1){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		waitCycle += 6;
	}
	
	if(opcode == 0xD1){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		cmp(value);
		
		waitCycle += 5;
		if(pageCrossed)
			waitCycle++;
	}

	if(opcode == 0xE0){
		uint8_t value = getModeInstruction(Immediate);
		cpx(value);
		waitCycle += 2;
	}

	if(opcode == 0xE4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cpx(value);
		waitCycle += 3;
	}

	if(opcode == 0xEC){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cpx(value);
		waitCycle += 4;
	}

	if(opcode == 0xC0){
		uint8_t value = getModeInstruction(Immediate);
		cpy(value);
		waitCycle += 2;
	}

	if(opcode == 0xC4){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		cpy(value);
		waitCycle += 3;
	}

	if(opcode == 0xCC){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		cpy(value);
		waitCycle += 4;
	}

	if(opcode == 0xE6){
		uint16_t address = getModeInstruction(ZeroPage);
		inc(address);
		waitCycle += 5;
	}

	if(opcode == 0xF6){
		uint16_t address = getModeInstruction(ZeroPageX);
		inc(address);
		waitCycle += 6;
	}

	if(opcode == 0xEE){
		uint16_t address = getModeInstruction(Absolute);
		inc(address);
		waitCycle += 6;
	}

	if(opcode == 0xFE){
		uint16_t address = getModeInstruction(AbsoluteX);
		inc(address);
		waitCycle += 7;
	}

	if(opcode == 0xE8){
		inx();
		waitCycle += 2;
	}

	if(opcode == 0xC8){
		iny();
		waitCycle += 2;
	}

	if(opcode == 0xC6){
		uint16_t address = getModeInstruction(ZeroPage);
		dec(address);
		waitCycle += 5;
	}

	if(opcode == 0xD6){
		uint16_t address = getModeInstruction(ZeroPageX);
		dec(address);
		waitCycle += 6;
	}

	if(opcode == 0xCE){
		uint16_t address = getModeInstruction(Absolute);
		dec(address);
		waitCycle += 6;
	}

	if(opcode == 0xDE){
		uint16_t address = getModeInstruction(AbsoluteX);
		dec(address);
		waitCycle += 7;
	}

	if(opcode == 0xCA){
		dex();
		waitCycle += 2;
	}

	if(opcode == 0x88){
		dey();
		waitCycle += 2;
	}

	if(opcode == 0x0A){
		asl(a, true);
		waitCycle += 2;
	}

	if(opcode == 0x06){
		uint16_t address = getModeInstruction(ZeroPage);
		asl(address, false);
		waitCycle += 5;
	}

	if(opcode == 0x16){
		uint16_t address = getModeInstruction(ZeroPageX);
		asl(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x0E){
		uint16_t address = getModeInstruction(Absolute);
		asl(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x1E){
		uint16_t address = getModeInstruction(AbsoluteX);
		asl(address, false);
		waitCycle += 7;
	}

	if(opcode == 0x4A){
		lsr(a, true);
		waitCycle += 2;
	}

	if(opcode == 0x46){
		uint16_t address = getModeInstruction(ZeroPage);
		lsr(address, false);
		waitCycle += 5;
	}

	if(opcode == 0x56){
		uint16_t address = getModeInstruction(ZeroPageX);
		lsr(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x4E){
		uint16_t address = getModeInstruction(Absolute);
		lsr(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x5E){
		uint16_t address = getModeInstruction(AbsoluteX);
		lsr(address, false);
		waitCycle += 7;
	}

	if(opcode == 0x2A){
		rol(a, true);
		waitCycle += 2;
	}

	if(opcode == 0x26){
		uint16_t address = getModeInstruction(ZeroPage);
		rol(address, false);
		waitCycle += 5;
	}

	if(opcode == 0x36){
		uint16_t address = getModeInstruction(ZeroPageX);
		rol(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x2E){
		uint16_t address = getModeInstruction(Absolute);
		rol(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x3E){
		uint16_t address = getModeInstruction(AbsoluteX);
		rol(address, false);
		waitCycle += 7;
	}

	if(opcode == 0x6A){
		ror(a, true);
		waitCycle += 2;
	}

	if(opcode == 0x66){
		uint16_t address = getModeInstruction(ZeroPage);
		ror(address, false);
		waitCycle += 5;
	}

	if(opcode == 0x76){
		uint16_t address = getModeInstruction(ZeroPageX);
		ror(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x6E){
		uint16_t address = getModeInstruction(Absolute);
		ror(address, false);
		waitCycle += 6;
	}

	if(opcode == 0x7E){
		uint16_t address = getModeInstruction(AbsoluteX);
		ror(address, false);
		waitCycle += 7;
	}

	if(opcode == 0x4C){
		uint16_t address = getModeInstruction(Absolute);
		jmp(address);
		waitCycle += 3;
	}

	if(opcode == 0x6C){
		uint16_t address = getModeInstruction(Indirect);
		jmp(address);
		waitCycle += 5;
	}

	if(opcode == 0x20){
		uint16_t address = getModeInstruction(Absolute);
		jsr(address);
		waitCycle += 6;
	}

	if(opcode == 0x60){
		rts();
		waitCycle += 6;
	}

	if(opcode == 0x90){
		uint8_t displacement = getModeInstruction(Relative);
		bcc(displacement);

		waitCycle += 2;
	}

	if(opcode == 0xB0){
		uint8_t displacement = getModeInstruction(Relative);
		bcs(displacement);

		waitCycle += 2;
	}

	if(opcode == 0xF0){
		uint8_t displacement = getModeInstruction(Relative);
		beq(displacement);

		waitCycle += 2;
	}

	if(opcode == 0x30){
		uint16_t displacement = getModeInstruction(Relative);
		bmi(displacement);

		waitCycle += 2;
	}

	if(opcode == 0xD0){
		uint16_t displacement = getModeInstruction(Relative);
		bne(displacement);

		waitCycle += 2;
	}

	if(opcode == 0x10){
		uint16_t displacement = getModeInstruction(Relative);
		bpl(displacement);

		waitCycle += 2;
	}

	if(opcode == 0x50){
		uint16_t displacement = getModeInstruction(Relative);
		bvc(displacement);

		waitCycle += 2;
	}

	if(opcode == 0x70){
		uint16_t displacement = getModeInstruction(Relative);
		bvs(displacement);

		waitCycle += 2;
	}

	if(opcode == 0x18){
		clc();
		waitCycle += 2;
	}

	if(opcode == 0xD8){
		cld();
		waitCycle += 2;
	}

	if(opcode == 0x58){
		cli();
		waitCycle += 2;
	}

	if(opcode == 0xB8){
		clv();
		waitCycle += 2;
	}

	if(opcode == 0x38){
		sec();
		waitCycle += 2;
	}

	if(opcode == 0xF8){
		sed();
		waitCycle += 2;
	}

	if(opcode == 0x78){
		sei();
		waitCycle += 2;
	}

	if(opcode == 0x00){
		brk();
		waitCycle += 7;
	}

	if(opcode == 0x40){
		rti();
		waitCycle += 6;
	}

	if(opcode == 0x9E){
	 	uint16_t address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		uint16_t temp = address + y;
		
		uint8_t ms = address >> 8;
		ms++;
		
		if((address & 0xFF00) != (temp & 0xFF00)){
			temp = address & 0x00FF | ((x & ms) << 8);
			temp += y;
		}

		stx(temp & ms);
		
		waitCycle += 5;
	}

	if(opcode == 0x9C){
	 	uint16_t address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
		uint16_t temp = address + x;

		uint8_t ms = address >> 8;
		ms++;
		
		if((address & 0xFF00) != (temp & 0xFF00)){
			temp = address & 0x00FF | ((y & ms) << 8);
			temp += x;
		}

		stx(temp & ms);

		waitCycle += 5;
	}

	if(opcode == 0xA7){
		uint16_t address = getModeInstruction(ZeroPage);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 3;
	}


	if(opcode == 0xB7){
		uint16_t address = getModeInstruction(ZeroPageY);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 4;
	}

	if(opcode == 0xAF){
		uint16_t address = getModeInstruction(Absolute);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 4;
	}
	
	if(opcode == 0xBF){
		uint16_t address = getModeInstruction(AbsoluteY);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}
	
	if(opcode == 0xA3){
		uint16_t address = getModeInstruction(IndexedIndirect);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 6;
	}
	
	if(opcode == 0xB3){
		uint16_t address = getModeInstruction(IndirectIndexed);
		uint8_t value = bus->cpuRead(address);
		lax(value);

		waitCycle += 4;
		
		if(pageCrossed)
			waitCycle++;
	}

	if( opcode == 0xE9 || 
		opcode == 0xEB
	){
		uint8_t value = getModeInstruction(Immediate);
		adc(value ^ 0xFF);
		waitCycle += 2;
	}

	if( opcode == 0x1A || 
		opcode == 0x3A || 
		opcode == 0x5A || 
		opcode == 0x7A || 
		opcode == 0xDA || 
		opcode == 0xEA || 
		opcode == 0xFA
	){
		nop();
		waitCycle += 2;
	}

	if( opcode == 0x80 ||
		opcode == 0x82 ||
		opcode == 0x89 ||
		opcode == 0xC2 ||
		opcode == 0xE2
	){

		getModeInstruction(Immediate);
		waitCycle += 2;
	}

	if(opcode == 0x0C){
		getModeInstruction(Absolute);
		waitCycle += 4;
	}

	if( opcode == 0x1C ||
		opcode == 0x3C ||
		opcode == 0x5C ||
		opcode == 0x7C ||
		opcode == 0xDC ||
		opcode == 0xFC
	){

		getModeInstruction(AbsoluteX);
		
		waitCycle += 4;

		if(pageCrossed)
			waitCycle++;
	}

	if( opcode == 0x04 || 
		opcode == 0x44 || 
		opcode == 0x64
	){	
		getModeInstruction(ZeroPage);
		waitCycle += 3;
	}

	if( opcode == 0x14 || 
		opcode == 0x34 || 
		opcode == 0x54 || 
		opcode == 0x74 || 
		opcode == 0xD4 || 
		opcode == 0xF4
	){
		getModeInstruction(ZeroPageX);
		waitCycle += 4;
	}



	pc++;
}
