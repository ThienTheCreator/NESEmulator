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

	uint16_t address;
	uint16_t temp;
	uint8_t displacement;
	
	switch(mode){
		case Immediate: 
			return bus->cpuRead(pc);
	
		case ZeroPage:
			return bus->cpuRead(pc) & 0xFF;
	
		case ZeroPageX:
			address = bus->cpuRead(pc);
			return (uint8_t)(address + x) & 0xFF;

	
		case ZeroPageY:
			address = bus->cpuRead(pc);
			return (uint8_t)(address + y) & 0xFF;
	
		case Relative:
			displacement = bus->cpuRead(pc);
			return displacement;
	
		case Absolute:
			address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			return address;
	
		case AbsoluteX:
		 	address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			temp = address;
			address += x;
			
			if((address & 0xFF00) != (temp & 0xFF00)){
				pageCrossed = true;
			} else {
				pageCrossed = false;
			}
			
			return address;

	
		case AbsoluteY:
			address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			temp = address;
			address += y;

			if((address & 0xFF00) != (temp & 0xFF00)){
				pageCrossed = true;
			} else {
				pageCrossed = false;
			}
		
			return address;

		case Indirect:
			address = bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			address = bus->cpuRead(address) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);

			return address;

		case IndexedIndirect:
			address = bus->cpuRead(pc);
			address = (address + x) & 0xFF;
			address = bus->cpuRead(address & 0xFF) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);
		
			return address;

		case IndirectIndexed:
			address = bus->cpuRead(pc);
			address = bus->cpuRead(address) | (bus->cpuRead(address & 0xFF00 | ((address + 1) & 0x00FF)) << 8);
		
			temp = address;
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
void CPU6502::ldy(uint8_t value){
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

void CPU6502::sax(uint16_t address){
	bus->cpuWrite(address, a & x);
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

		if(false){ // debug
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
	uint16_t address;
	uint16_t temp;
	uint8_t displacement;
	uint8_t value;
	
	switch(opcode){
		case 0xA9:
			value = getModeInstruction(Immediate);
			lda(value);
			waitCycle += 2;
			break;

	
		case 0xA5:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			lda(value);
			waitCycle += 3;
			break;

	
		case 0xB5:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			lda(value);
			waitCycle += 4;
			break;

	
		case 0xAD:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			lda(value);
			waitCycle += 4;
			break;

	
		case 0xBD:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			lda(value);
			
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xB9:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			lda(value);
			
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xA1:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			lda(value);
	
			waitCycle += 6;
			break;

	
		case 0xB1:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			lda(value);
			
			waitCycle += 5;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xA2:
			value = getModeInstruction(Immediate);
			ldx(value);
	
			waitCycle += 2;
			break;

	
		case 0xA6:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			ldx(value);
	
			waitCycle += 3;
			break;

	
		case 0xB6:
			address = getModeInstruction(ZeroPageY);
			value = bus->cpuRead(address);
			ldx(value);
	
			waitCycle += 4;
			break;

	
		case 0xAE:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			ldx(value);
	
			waitCycle += 4;
			break;

	
		case 0xBE:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			ldx(value);
		
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xA0:
			value = getModeInstruction(Immediate);
			ldy(value);
	
			waitCycle += 2;
			break;

	
		case 0xA4:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			ldy(value);
	
			waitCycle += 3;
			break;

	
		case 0xB4:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			ldy(value);
			
			waitCycle += 4;
			break;

	
		case 0xAC:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			ldy(value);
			
			waitCycle += 4;
			break;

	
		case 0xBC:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			ldy(value);
		
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x85:
			address = getModeInstruction(ZeroPage);
			sta(address);
			waitCycle += 3;
			break;

	
		case 0x95:
			address = getModeInstruction(ZeroPageX);
			sta(address);
			waitCycle += 4;
			break;

	
		case 0x8D:
			address = getModeInstruction(Absolute);
			sta(address);
			waitCycle += 4;
			break;

	
		case 0x9D:
			address = getModeInstruction(AbsoluteX);
			sta(address);
			waitCycle += 5;
			break;

	
		case 0x99:
			address = getModeInstruction(AbsoluteY);
			sta(address);
			waitCycle += 5;
			break;

	
		case 0x81:
			address = getModeInstruction(IndexedIndirect);
			sta(address);
			waitCycle += 6;
			break;

	
		case 0x91:
			address = getModeInstruction(IndirectIndexed);
			sta(address);
			waitCycle += 6;
			break;

	
		case 0x86:
			address = getModeInstruction(ZeroPage);
			stx(address);
			waitCycle += 3;
			break;

	
		case 0x96:
			address = getModeInstruction(ZeroPageY);
			stx(address);
			waitCycle += 4;
			break;

	
		case 0x8E:
			address = getModeInstruction(Absolute);
			stx(address);
			waitCycle += 4;
			break;

	
		case 0x84:
			address = getModeInstruction(ZeroPage);
			sty(address);
			waitCycle += 3;
			break;

	
		case 0x94:
			address = getModeInstruction(ZeroPageX);
			sty(address);
			waitCycle += 4;
			break;

	
		case 0x8C:
			address = getModeInstruction(Absolute);
			sty(address);
			waitCycle += 4;
			break;

	
		case 0xAA:
			tax();
			waitCycle += 2;
			break;

	
		case 0xA8:
			tay();
			waitCycle += 2;
			break;

	
		case 0x8A:
			txa();
			waitCycle += 2;
			break;

	
		case 0x98:
			tya();
			waitCycle += 2;
			break;

	
		case 0xBA:
			tsx();
			waitCycle += 2;
			break;

	
		case 0x9A:
			txs();
			waitCycle += 2;
			break;

	
		case 0x48:
			pha();
			waitCycle += 3;
			break;

	
		case 0x08:
			php();
			waitCycle += 3;
			break;

	
		case 0x68:
			pla();
			waitCycle += 4;
			break;

	
		case 0x28:
			plp();
			waitCycle += 4;
			break;

	
		case 0x29:
			value = getModeInstruction(Immediate);
			andL(value);
			waitCycle += 2;
			break;

	
		case 0x25:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			andL(value);
			waitCycle += 3;
			break;

	
		case 0x35:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			andL(value);
			waitCycle += 4;
			break;

	
		case 0x2D:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			andL(value);
			waitCycle += 4;
			break;

	
		case 0x3D:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			andL(value);
	
			waitCycle += 4;
	
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x39:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			andL(value);
		
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x21:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			andL(value);
	
			waitCycle += 6;
			break;

	
		case 0x31:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			andL(value);
		
			waitCycle += 5;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x49:
			value = getModeInstruction(Immediate);
			eor(value);
	
			waitCycle += 2;
			break;

	
		case 0x45:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			eor(value);
			waitCycle += 3;
			break;

	
		case 0x55:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			eor(value);
			waitCycle += 4;
			break;

	
		case 0x4D:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			eor(value);
			waitCycle += 4;
			break;

	
		case 0x5D:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			eor(value);
		
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x59:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			eor(value);
		
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x41:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			eor(value);
	
			waitCycle += 6;
			break;

	
		case 0x51:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			eor(value);
		
			waitCycle += 5;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x09:
			value = getModeInstruction(Immediate);
			ora(value);
	
			waitCycle += 2;
			break;

	
		case 0x05:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			ora(value);
			waitCycle += 3;
			break;

	
		case 0x15:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			ora(value);
			waitCycle += 4;
			break;

	
		case 0x0D:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			ora(value);
			waitCycle += 4;
			break;

	
		case 0x1D:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			ora(value);
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x19:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			ora(value);
			
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x01:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			ora(value);
	
			waitCycle += 6;
			break;

	
		case 0x11:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			ora(value);
			
			waitCycle += 5;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x24:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			bit(value);
			waitCycle += 3;
			break;

	
		case 0x2C:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			bit(value);
			waitCycle += 4;
			break;

	
		case 0x69:
			value = getModeInstruction(Immediate);
			adc(value);
			waitCycle += 2;
			break;

	
		case 0x65:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			adc(value);
			waitCycle += 3;
			break;

	
		case 0x75:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			adc(value);
			waitCycle += 4;
			break;

	
		case 0x6D:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			adc(value);
			waitCycle += 4;
			break;

	
		case 0x7D:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			adc(value);
			
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x79:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			adc(value);
	
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x61:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			adc(value);
			waitCycle += 6;
			break;

	
		case 0x71:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			adc(value);
	
			waitCycle += 5;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xE5:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			sbc(value);
			waitCycle += 3;
			break;

	
		case 0xF5:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			sbc(value);
			waitCycle += 4;
			break;

	
		case 0xED:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			sbc(value);
			waitCycle += 4;
			break;

	
		case 0xFD:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			sbc(value);
			
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xF9:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			sbc(value);
		
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xE1:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			sbc(value);
			
			waitCycle += 6;
			break;

	
		case 0xF1:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			sbc(value);
	
			waitCycle += 5;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xC9:
			value = getModeInstruction(Immediate);
			cmp(value);
			waitCycle += 2;
			break;

	
		case 0xC5:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			cmp(value);
			waitCycle += 3;
			break;

	
		case 0xD5:
			address = getModeInstruction(ZeroPageX);
			value = bus->cpuRead(address);
			cmp(value);
			waitCycle += 4;
			break;

	
		case 0xCD:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			cmp(value);
			waitCycle += 4;
			break;

		
		case 0xDD:
			address = getModeInstruction(AbsoluteX);
			value = bus->cpuRead(address);
			cmp(value);
	
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

		
		case 0xD9:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			cmp(value);
	
			waitCycle += 4;
			if(pageCrossed)
				waitCycle++;
			break;

		
		case 0xC1:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			cmp(value);
			waitCycle += 6;
			break;

		
		case 0xD1:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			cmp(value);
			
			waitCycle += 5;
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xE0:
			value = getModeInstruction(Immediate);
			cpx(value);
			waitCycle += 2;
			break;

	
		case 0xE4:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			cpx(value);
			waitCycle += 3;
			break;

	
		case 0xEC:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			cpx(value);
			waitCycle += 4;
			break;

	
		case 0xC0:
			value = getModeInstruction(Immediate);
			cpy(value);
			waitCycle += 2;
			break;

	
		case 0xC4:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			cpy(value);
			waitCycle += 3;
			break;

	
		case 0xCC:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			cpy(value);
			waitCycle += 4;
			break;

	
		case 0xE6:
			address = getModeInstruction(ZeroPage);
			inc(address);
			waitCycle += 5;
			break;

	
		case 0xF6:
			address = getModeInstruction(ZeroPageX);
			inc(address);
			waitCycle += 6;
			break;

	
		case 0xEE:
			address = getModeInstruction(Absolute);
			inc(address);
			waitCycle += 6;
			break;

	
		case 0xFE:
			address = getModeInstruction(AbsoluteX);
			inc(address);
			waitCycle += 7;
			break;

	
		case 0xE8:
			inx();
			waitCycle += 2;
			break;

	
		case 0xC8:
			iny();
			waitCycle += 2;
			break;

	
		case 0xC6:
			address = getModeInstruction(ZeroPage);
			dec(address);
			waitCycle += 5;
			break;

	
		case 0xD6:
			address = getModeInstruction(ZeroPageX);
			dec(address);
			waitCycle += 6;
			break;

	
		case 0xCE:
			address = getModeInstruction(Absolute);
			dec(address);
			waitCycle += 6;
			break;

	
		case 0xDE:
			address = getModeInstruction(AbsoluteX);
			dec(address);
			waitCycle += 7;
			break;

	
		case 0xCA:
			dex();
			waitCycle += 2;
			break;

	
		case 0x88:
			dey();
			waitCycle += 2;
			break;

	
		case 0x0A:
			asl(a, true);
			waitCycle += 2;
			break;

	
		case 0x06:
			address = getModeInstruction(ZeroPage);
			asl(address, false);
			waitCycle += 5;
			break;

	
		case 0x16:
			address = getModeInstruction(ZeroPageX);
			asl(address, false);
			waitCycle += 6;
			break;

	
		case 0x0E:
			address = getModeInstruction(Absolute);
			asl(address, false);
			waitCycle += 6;
			break;

	
		case 0x1E:
			address = getModeInstruction(AbsoluteX);
			asl(address, false);
			waitCycle += 7;
			break;

	
		case 0x4A:
			lsr(a, true);
			waitCycle += 2;
			break;

	
		case 0x46:
			address = getModeInstruction(ZeroPage);
			lsr(address, false);
			waitCycle += 5;
			break;

	
		case 0x56:
			address = getModeInstruction(ZeroPageX);
			lsr(address, false);
			waitCycle += 6;
			break;

	
		case 0x4E:
			address = getModeInstruction(Absolute);
			lsr(address, false);
			waitCycle += 6;
			break;

	
		case 0x5E:
			address = getModeInstruction(AbsoluteX);
			lsr(address, false);
			waitCycle += 7;
			break;

	
		case 0x2A:
			rol(a, true);
			waitCycle += 2;
			break;

	
		case 0x26:
			address = getModeInstruction(ZeroPage);
			rol(address, false);
			waitCycle += 5;
			break;

	
		case 0x36:
			address = getModeInstruction(ZeroPageX);
			rol(address, false);
			waitCycle += 6;
			break;

	
		case 0x2E:
			address = getModeInstruction(Absolute);
			rol(address, false);
			waitCycle += 6;
			break;

	
		case 0x3E:
			address = getModeInstruction(AbsoluteX);
			rol(address, false);
			waitCycle += 7;
			break;

	
		case 0x6A:
			ror(a, true);
			waitCycle += 2;
			break;

	
		case 0x66:
			address = getModeInstruction(ZeroPage);
			ror(address, false);
			waitCycle += 5;
			break;

	
		case 0x76:
			address = getModeInstruction(ZeroPageX);
			ror(address, false);
			waitCycle += 6;
			break;

	
		case 0x6E:
			address = getModeInstruction(Absolute);
			ror(address, false);
			waitCycle += 6;
			break;

	
		case 0x7E:
			address = getModeInstruction(AbsoluteX);
			ror(address, false);
			waitCycle += 7;
			break;

	
		case 0x4C:
			address = getModeInstruction(Absolute);
			jmp(address);
			waitCycle += 3;
			break;

	
		case 0x6C:
			address = getModeInstruction(Indirect);
			jmp(address);
			waitCycle += 5;
			break;

	
		case 0x20:
			address = getModeInstruction(Absolute);
			jsr(address);
			waitCycle += 6;
			break;

	
		case 0x60:
			rts();
			waitCycle += 6;
			break;

	
		case 0x90:
			displacement = getModeInstruction(Relative);
			bcc(displacement);
	
			waitCycle += 2;
			break;

	
		case 0xB0:
			displacement = getModeInstruction(Relative);
			bcs(displacement);
	
			waitCycle += 2;
			break;

	
		case 0xF0:
			displacement = getModeInstruction(Relative);
			beq(displacement);
	
			waitCycle += 2;
			break;

	
		case 0x30:
			displacement = getModeInstruction(Relative);
			bmi(displacement);
	
			waitCycle += 2;
			break;

	
		case 0xD0:
			displacement = getModeInstruction(Relative);
			bne(displacement);
	
			waitCycle += 2;
			break;

	
		case 0x10:
			displacement = getModeInstruction(Relative);
			bpl(displacement);
	
			waitCycle += 2;
			break;

	
		case 0x50:
			displacement = getModeInstruction(Relative);
			bvc(displacement);
	
			waitCycle += 2;
			break;

	
		case 0x70:
			displacement = getModeInstruction(Relative);
			bvs(displacement);
	
			waitCycle += 2;
			break;

	
		case 0x18:
			clc();
			waitCycle += 2;
			break;

	
		case 0xD8:
			cld();
			waitCycle += 2;
			break;

	
		case 0x58:
			cli();
			waitCycle += 2;
			break;

	
		case 0xB8:
			clv();
			waitCycle += 2;
			break;

	
		case 0x38:
			sec();
			waitCycle += 2;
			break;

	
		case 0xF8:
			sed();
			waitCycle += 2;
			break;

	
		case 0x78:
			sei();
			waitCycle += 2;
			break;

	
		case 0x00:
			brk();
			waitCycle += 7;
			break;

	
		case 0x40:
			rti();
			waitCycle += 6;
			break;

	
		case 0x9E:
		 	address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			temp = address + y;
			{
				uint8_t ms = address >> 8;
				ms++;
				
				if((address & 0xFF00) != (temp & 0xFF00)){
					temp = address & 0x00FF | ((x & ms) << 8);
					temp += y;
				}
	
				stx(temp & ms);
			}
			waitCycle += 5;
			break;

	
		case 0x9C:
		 	address = (uint8_t)bus->cpuRead(pc) | (bus->cpuRead(++pc) << 8);
			temp = address + x;

			{
				uint8_t ms = address >> 8;
				ms++;
			
				if((address & 0xFF00) != (temp & 0xFF00)){
					temp = address & 0x00FF | ((y & ms) << 8);
					temp += x;
				}
	
				stx(temp & ms);
			}
			waitCycle += 5;
			break;

	
		case 0xA7:
			address = getModeInstruction(ZeroPage);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 3;
			break;

	
	
		case 0xB7:
			address = getModeInstruction(ZeroPageY);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 4;
			break;

	
		case 0xAF:
			address = getModeInstruction(Absolute);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 4;
			break;

		
		case 0xBF:
			address = getModeInstruction(AbsoluteY);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0x87:
			address = getModeInstruction(ZeroPage);
			sax(address);
			break;

	
		case 0x97:
			address = getModeInstruction(ZeroPageY);
			sax(address);
			break;

	
		case 0x8F:
			address = getModeInstruction(Absolute);
			sax(address);
			break;

	
		case 0x83:
			address = getModeInstruction(IndexedIndirect);
			sax(address);
			break;

		
		case 0xA3:
			address = getModeInstruction(IndexedIndirect);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 6;
			break;

		
		case 0xB3:
			address = getModeInstruction(IndirectIndexed);
			value = bus->cpuRead(address);
			lax(value);
	
			waitCycle += 4;
			
			if(pageCrossed)
				waitCycle++;
			break;

	
		case 0xE9: case 0xEB:
			value = getModeInstruction(Immediate);
			adc(value ^ 0xFF);
			waitCycle += 2;
			break;
	
		case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xEA: case 0xFA:
			nop();
			waitCycle += 2;
			break;
	
		case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2:
			getModeInstruction(Immediate);
			waitCycle += 2;
			break;
	
		case 0x0C:
			getModeInstruction(Absolute);
			waitCycle += 4;
			break;

	
		case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC:
			getModeInstruction(AbsoluteX);
			
			waitCycle += 4;
	
			if(pageCrossed)
				waitCycle++;
			break;
	
		case 0x04: case 0x44: case 0x64:	
			getModeInstruction(ZeroPage);
			waitCycle += 3;
			break;
	
		case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4:
			getModeInstruction(ZeroPageX);
			waitCycle += 4;
			break;
	}


	pc++;
}
