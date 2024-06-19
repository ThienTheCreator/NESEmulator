#pragma once

#include <iostream>
#include <cstdint>

class Bus;

class CPU6502{
	Bus *bus = nullptr;
public:
	// CPU registers
	uint8_t  a  = 0;    // Accumulator
	uint8_t  x  = 0;    // Index X
	uint8_t  y  = 0;    // Index Y
	uint16_t pc = 0;    // Program Counter
	uint8_t  s  = 0xFF; // Stack Pointer
	
	uint8_t p = 0; // status register

	CPU6502(){
		pc = 0xFFFC;
		s = 0xFD;

		p = 0x24;
	}

	void connectBus(Bus* b){ bus = b; }

	void setFlag(uint8_t bit);

	void unsetFlag(uint8_t bit);

	void handleFlag(uint8_t flag, bool expression);

	bool getFlag(uint8_t bit);


	void irq();

	void nmi();

	void reset();

	/* 
	** Load or Store Operations 
	*/

	// Load Accumulator
	void lda(uint8_t address);

	// Load X Register
	void ldx(uint8_t address);

	// Load Y Register
	void ldy(uint16_t address);

	// Store Accumulator
	void sta(uint16_t address);

	// Store X Register
	void stx(uint16_t address);

	// Store Y Register
	void sty(uint16_t address);

	/*
	** Register Transfers
	*/

	// Transfer Accumulator to X
	void tax();

	// Transfer Accumulator to Y
	void tay();

	// Transfer X to Accumulator
	void txa();

	// Transfer Y to Accumulator
	void tya();

	/*
	** Stack Operation
	*/

	// Transfer Stack Pointer to X
	void tsx();

	// Transfer X to Stack Pointer
	void txs();

	// Push Accumulator
	void pha();

	// Push Processor Status
	void php();

	// Pull Accumulator
	void pla();

	// Pull Processor Status
	void plp();

	/*
	** Logical 
	*/

	// Logical and
	void andL(uint8_t value);
	
	// Exclusive OR
	void eor(uint8_t value);

	// Logical Inclusive OR
	void ora(uint8_t value);

	// bit test
	void bit(uint8_t value);

	/*
	** Arithmetic
	*/

	// ADC - Add with Carry
	void adc(uint8_t value);

	// Subtract with Carry
	void sbc(uint8_t value);

	// Compare
	void cmp(uint8_t value);

	// Compare X Register
	void cpx(uint8_t value);

	// Compare Y Register
	void cpy(uint8_t value);

	/*
	** Increment and Decrements
	*/

	// Increment Memory
	void inc(uint16_t address);

	// Increment X Register
	void inx();

	// Increment Y Register
	void iny();

	// Decrement Memory
	void dec(uint16_t address);

	// Decrement X Register
	void dex();

	// Decrement Y Register
	void dey();

	/*
	** Shifts
	*/

	// Arithmetic Shift Left
	void asl(uint16_t address, bool isAccumulatorMode);

	// Logical Shift Right
	void lsr(uint16_t address, bool isAccumulatorMode);

	// Rotate Left
	void rol(uint16_t address, bool isAccumulatorMode);

	// Rotate Right
	void ror(uint16_t address, bool isAccumulatorMode);

	/*
	** Jumps and Calls
	*/

	// Jump
	void jmp(uint16_t address);

	// Jump to Subroutine
	void jsr(uint16_t address);

	// Return from Subroutine
	void rts();

	/*
	** Branches
	*/

	// Branch if Carry Clear
	void bcc(int8_t displacement);

	// Branch if Carry Set
	void bcs(int8_t displacement);

	// Branch if Equal
	void beq(int8_t displacement);


	// Branch if Minus
	void bmi(int8_t displacement);

	// Branch if Not Equal
	void bne(int8_t displacement);

	// Branch if Positive
	void bpl(int8_t displacement);

	// Branch if Overflow Clear
	void bvc(int8_t displacement);

	// Branch if Overflow Set
	void bvs(int8_t displacement);

	/*
	** Status Flag Changes
	*/

	// Clear Carry Flag
	void clc();

	// Clear Decimal Mode
	void cld();

	// Clear Interrupt Disable
	void cli();

	// Clear Overflow Flag
	void clv();

	// Set Carry Flag
	void sec();

	// Set Decimal Flag
	void sed();

	// Set Interrupt Disable
	void sei();
	
	/*
	** System Functions
	*/

	// Force an Interrupt
	void brk();

	// No Operation
	void nop();
		
	// Return from Interrupt
	void rti();

	uint16_t getModeInstruction(int);

	void executeInstruction(uint8_t opcode);
};
