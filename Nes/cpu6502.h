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
	uint8_t  s  = 0xFF; // Stack Pointeri
	
	uint8_t p = 0b01100000; // status register

	void connectBus(Bus* b){ bus = b; }

	void setFlag(uint8_t bit);

	void unsetFlag(uint8_t bit);

	void handleFlag(uint8_t flag, bool expression);

	bool getFlag(uint8_t bit);

	/* 
	** Load or Store Operations 
	*/

	// Load Accumulator
	void lda(uint16_t address);

	// Load X Register
	void ldx(uint16_t address);

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
	void andL(uint16_t address);
	
	// Exclusive OR
	void eor(uint16_t address);

	// Logical Inclusive OR
	void ora(uint16_t address);

	// bit test
	void bit(uint16_t address);

	/*
	** Arithmetic
	*/

	// ADC - Add with Carry
	void adc(uint16_t address);

	// Subtract with Carry
	void sbc(uint16_t address);

	// Compare
	void cmp(uint16_t address);

	// Compare X Register
	void cpx(uint16_t address);

	// Compare Y Register
	void cpy(uint16_t address);

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
	void asl(uint16_t address);

	// Logical Shift Right
	void lsr();

	// Rotate Left
	void rol();

	// Rotate Right
	void ror();

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
	void bcc(uint16_t displacement);

	// Branch if Carry Set
	void bcs(uint16_t displacement);

	// Branch if Equal
	void beq(uint16_t displacement);


	// Branch if Minus
	void bmi(uint16_t displacement);

	// Branch if Not Equal
	void bne(uint16_t displacement);

	// Branch if Positive
	void bpl(uint16_t displacement);

	// Branch if Overflow Clear
	void bvc(uint16_t displacement);

	// Branch if Overflow Set
	void bvs(uint16_t displacement);

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

	void irq();

	void nmi();

	uint16_t handleMode(int, uint16_t);

	void executeTest(uint8_t opcode);
};
