/*

	2A03 disassembler.
	Debugging tools

	(c) 2023 P. Santing
	

*/

#pragma once

#include <stdint.h>
#include "../../bus/bus.h"

enum operand_types {
	acc, absolute, absx, absy, imm, impl, ind, xind, indy, rel, zpg, zpgx, zpgy
};

typedef struct {
	uint8_t			opcode;
	operand_types	optype;
	char			mnemonic[5];
} opcode;

static const opcode opcode_table[] = {
	{0x00, operand_types::impl, "BRK "}, 	{0x01, operand_types::xind, "ORA "},
	{0x02, operand_types::impl, "ILL "},		{0x03, operand_types::impl, "ILL "},
	{0x04, operand_types::impl, "ILL "},		{0x05, operand_types::zpg, "ORA "},
	{0x06, operand_types::zpg, "ASL "},		{0x07, operand_types::impl, "ILL "},
	{0x08, operand_types::impl, "PHP "},		{0x09, operand_types::imm, "ORA "},
	{0x0A, operand_types::acc, "ASL "},		{0x0B, operand_types::impl, "ILL "},
	{0x0C, operand_types::impl, "ILL "},		{0x0D, operand_types::absolute, "ORA "},
	{0x0E, operand_types::absolute, "ASL "},		{0x0F, operand_types::impl, "ILL "},
	{0x10, operand_types::rel, "BPL "},		{0x11, operand_types::indy, "ORA "},
	{0x12, operand_types::impl, "ILL "},		{0x13, operand_types::impl, "ILL "},
	{0x14, operand_types::impl, "ILL "},		{0x15, operand_types::zpgx, "ORA "},
	{0x16, operand_types::zpgx, "ASL "},		{0x17, operand_types::impl, "ILL "},
	{0x18, operand_types::impl, "CLC "},		{0x19, operand_types::absy, "ORA "},
	{0x1A, operand_types::impl, "ILL "},		{0x1B, operand_types::impl, "ILL "},
	{0x1C, operand_types::impl, "ILL "},		{0x1D, operand_types::absx, "ORA "},
	{0x1E, operand_types::absx, "ASL "},		{0x1F, operand_types::impl, "ILL "},
	{0x20, operand_types::absolute, "JSR "},		{0x21, operand_types::xind, "AND "},
	{0x22, operand_types::impl, "ILL "},		{0x23, operand_types::impl, "ILL "},
	{0x24, operand_types::zpg, "BIT "},		{0x25, operand_types::zpg, "AND "},
	{0x26, operand_types::zpg, "ROL "},		{0x27, operand_types::impl, "ILL "},
	{0x28, operand_types::impl, "PLP "},		{0x29, operand_types::imm, "AND "},
	{0x2A, operand_types::acc, "ROL "},		{0x2B, operand_types::impl, "ILL "},
	{0x2C, operand_types::absolute, "BIT "},		{0x2D, operand_types::absolute, "AND "},
	{0x2E, operand_types::absolute, "ROL "},		{0x2F, operand_types::impl, "ILL "},
	{0x30, operand_types::rel, "BMI "},		{0x31, operand_types::indy, "AND "},
	{0x32, operand_types::impl, "ILL "},		{0x33, operand_types::impl, "ILL "},
	{0x34, operand_types::impl, "ILL "},		{0x35, operand_types::zpgx, "AND "},
	{0x36, operand_types::zpgx, "ROL "},		{0x37, operand_types::impl, "ILL "},
	{0x38, operand_types::impl, "SEC "},		{0x39, operand_types::absy, "AND "},
	{0x3A, operand_types::impl, "ILL "},		{0x3B, operand_types::impl, "ILL "},
	{0x3C, operand_types::impl, "ILL "},		{0x3D, operand_types::absx, "AND "},
	{0x3E, operand_types::absx, "ROL "},		{0x3F, operand_types::impl, "ILL "},
	{0x40, operand_types::impl, "RTI "},		{0x41, operand_types::xind, "EOR "},
	{0x42, operand_types::impl, "ILL "},		{0x43, operand_types::impl, "ILL "},
	{0x44, operand_types::impl, "ILL "},		{0x45, operand_types::zpg, "EOR "},
	{0x46, operand_types::zpg, "LSR "},		{0x47, operand_types::impl, "ILL "},
	{0x48, operand_types::impl, "PHA "},		{0x49, operand_types::imm, "EOR "},
	{0x4A, operand_types::acc, "LSR "},		{0x4B, operand_types::impl, "ILL "},
	{0x4C, operand_types::absolute, "JMP "},		{0x4D, operand_types::absolute, "EOR "},
	{0x4E, operand_types::absolute, "LSR "},		{0x4F, operand_types::impl, "ILL "},
	{0x50, operand_types::rel, "BVC "},		{0x51, operand_types::indy, "EOR "},
	{0x52, operand_types::impl, "ILL "},		{0x53, operand_types::impl, "ILL "},
	{0x54, operand_types::impl, "ILL "},		{0x55, operand_types::zpgx, "EOR "},
	{0x56, operand_types::zpgx, "LSR "},		{0x57, operand_types::impl, "ILL "},
	{0x58, operand_types::impl, "CLI "},		{0x59, operand_types::absy, "EOR "},
	{0x5A, operand_types::impl, "ILL "},		{0x5B, operand_types::impl, "ILL "},
	{0x5C, operand_types::impl, "ILL "},		{0x5D, operand_types::absx, "EOR "},
	{0x5E, operand_types::absx, "LSR "},		{0x5F, operand_types::impl, "ILL "},
	{0x60, operand_types::impl, "RTS "},		{0x61, operand_types::xind, "ADC "},
	{0x62, operand_types::impl, "ILL "},		{0x63, operand_types::impl, "ILL "},
	{0x64, operand_types::impl, "ILL "},		{0x65, operand_types::zpg, "ADC "},
	{0x66, operand_types::zpg, "ROR "},		{0x67, operand_types::impl, "ILL "},
	{0x68, operand_types::impl, "PLA "},		{0x69, operand_types::imm, "ADC "},
	{0x6A, operand_types::acc, "ROR "},		{0x6B, operand_types::impl, "ILL "},
	{0x6C, operand_types::ind, "JMP "},		{0x6D, operand_types::absolute, "ADC "},
	{0x6E, operand_types::absolute, "ROR "},		{0x6F, operand_types::impl, "ILL "},
	{0x70, operand_types::rel, "BVS "},		{0x71, operand_types::indy, "ADC "},
	{0x72, operand_types::impl, "ILL "},		{0x73, operand_types::impl, "ILL "},
	{0x74, operand_types::impl, "ILL "},		{0x75, operand_types::zpgx, "ADC "},
	{0x76, operand_types::zpgx, "ROR "},		{0x77, operand_types::impl, "ILL "},
	{0x78, operand_types::impl, "SEI "},		{0x79, operand_types::absy, "ADC "},
	{0x7A, operand_types::impl, "ILL "},		{0x7B, operand_types::impl, "ILL "},
	{0x7C, operand_types::impl, "ILL "},		{0x7D, operand_types::absx ,"ADC "},
	{0x7E, operand_types::absx, "ROR "},		{0x7F, operand_types::impl, "ILL "},
	{0x80, operand_types::impl, "ILL "},		{0x81, operand_types::xind, "STA "},
	{0x82, operand_types::impl, "ILL "},		{0x83, operand_types::impl, "ILL "},
	{0x84, operand_types::zpg, "STY "},		{0x85, operand_types::zpg, "STA "},
	{0x86, operand_types::zpg, "STX "},		{0x87, operand_types::impl, "ILL "},
	{0x88, operand_types::impl, "DEY "},		{0x89, operand_types::impl, "ILL "},
	{0x8A, operand_types::impl, "TXA "},		{0x8B, operand_types::impl, "ILL "},
	{0x8C, operand_types::absolute, "STY "},		{0x8D, operand_types::absolute, "STA "},
	{0x8E, operand_types::absolute, "STX "},		{0x8F, operand_types::impl, "ILL "},
	{0x90, operand_types::rel, "BCC "},		{0x91, operand_types::indy, "STA "},
	{0x92, operand_types::impl, "ILL "},		{0x93, operand_types::impl, "ILL "},
	{0x94, operand_types::zpgx, "STY "},		{0x95, operand_types::zpgx, "STA "},
	{0x96, operand_types::zpgx, "STX "},		{0x97, operand_types::impl, "ILL "},
	{0x98, operand_types::impl, "TYA "},		{0x99, operand_types::absy, "STA "},
	{0x9A, operand_types::impl, "TXS "},		{0x9B, operand_types::impl, "ILL "},
	{0x9C, operand_types::impl, "ILL "},		{0x9D, operand_types::absx, "STA "},
	{0x9E, operand_types::impl, "ILL "},		{0x9F, operand_types::impl, "ILL "},
	{0xA0, operand_types::imm, "LDY "},		{0xA1, operand_types::xind, "LDA "},
	{0xA2, operand_types::imm, "LDX "},		{0xA3, operand_types::impl, "ILL "},
	{0xA4, operand_types::zpg, "LDY "},		{0xA5, operand_types::zpg, "LDA "},
	{0xA6, operand_types::zpg, "LDX "},		{0xA7, operand_types::impl, "ILL "},
	{0xA8, operand_types::impl, "TAY "},		{0xA9, operand_types::imm, "LDA "},
	{0xAA, operand_types::impl, "TAX "},		{0xAB, operand_types::impl, "ILL "},
	{0xAC, operand_types::absolute, "LDY "},		{0xAD, operand_types::absolute, "LDA "},
	{0xAE, operand_types::absolute, "LDX "},		{0xAF, operand_types::impl, "ILL "},
	{0xB0, operand_types::rel, "BCS "},		{0xB1, operand_types::indy, "LDA "},
	{0xB2, operand_types::impl, "ILL "},		{0xB3, operand_types::impl, "ILL "},
	{0xB4, operand_types::zpgx, "LDY "},		{0xB5, operand_types::zpgx, "LDA "},
	{0xB6, operand_types::zpgy, "LDX "},		{0xB7, operand_types::impl, "ILL "},
	{0xB8, operand_types::impl, "CLV "},		{0xB9, operand_types::absy, "LDA "},
	{0xBA, operand_types::impl, "TSX "},		{0xBB, operand_types::impl, "ILL "},
	{0xBC, operand_types::absx, "LDY "},		{0xBD, operand_types::absx, "LDA "},
	{0xBE, operand_types::absy, "LDX "},		{0xBF, operand_types::impl, "ILL "},
	{0xC0, operand_types::imm, "CPY "},		{0xC1, operand_types::xind, "CMP "},
	{0xC2, operand_types::impl, "ILL "},		{0xC3, operand_types::impl, "ILL "},
	{0xC4, operand_types::zpg, "CPY "},		{0xC5, operand_types::zpg, "CMP "},
	{0xC6, operand_types::zpg, "DEC "},		{0xC7, operand_types::impl, "ILL "},
	{0xC8, operand_types::impl, "INY "},		{0xC9, operand_types::imm, "CMP "},
	{0xCA, operand_types::impl, "DEX "},		{0xCB, operand_types::impl, "ILL "},
	{0xCC, operand_types::absolute, "CPY "},		{0xCD, operand_types::absolute, "CMP "},
	{0xCE, operand_types::absolute, "DEC "},		{0xCF, operand_types::impl, "ILL "},
	{0xD0, operand_types::rel, "BNE "},		{0xD1, operand_types::indy, "CMP "},
	{0xD2, operand_types::impl, "ILL "},		{0xD3, operand_types::impl, "ILL "},
	{0xD4, operand_types::impl, "ILL "},		{0xD5, operand_types::zpgx, "CMP "},
	{0xD6, operand_types::zpgx, "DEC "},		{0xD7, operand_types::impl, "ILL "},
	{0xD8, operand_types::impl, "CLD "},		{0xD9, operand_types::absy, "CMP "},
	{0xDA, operand_types::impl, "ILL "},		{0xDB, operand_types::impl, "ILL "},
	{0xDC, operand_types::impl, "ILL "},		{0xDD, operand_types::absx, "CMP "},
	{0xDE, operand_types::absx, "DEC "},		{0xDF, operand_types::impl, "ILL "},
	{0xE0, operand_types::imm, "CPX "},		{0xE1, operand_types::xind, "SBC "},
	{0xE2, operand_types::impl, "ILL "},		{0xE3, operand_types::impl, "ILL "},
	{0xE4, operand_types::zpg, "CPX "},		{0xE5, operand_types::zpg, "SBC "},
	{0xE6, operand_types::zpg, "INC "},		{0xE7, operand_types::impl, "ILL "},
	{0xE8, operand_types::impl, "INX "},		{0XE9, operand_types::imm, "SBC "},
	{0xEA, operand_types::impl, "NOP "},		{0xEB, operand_types::impl, "ILL "},
	{0xEC, operand_types::absolute, "CPX "},		{0xED, operand_types::absolute, "SBC "},
	{0xEE, operand_types::absolute, "INC "},		{0xEF, operand_types::impl, "ILL "},
	{0xF0, operand_types::rel, "BEQ "},		{0xF1, operand_types::indy, "SBC "},
	{0xF2, operand_types::impl, "ILL "},		{0xF3, operand_types::impl, "ILL "},
	{0xF4, operand_types::impl, "ILL "},		{0xF5, operand_types::zpgx, "SBC "},
	{0xF6, operand_types::zpgx, "INC "},		{0xF7, operand_types::impl, "ILL "},
	{0xF8, operand_types::impl, "SED "},		{0xF9, operand_types::absy, "SBC "},
	{0xFA, operand_types::impl, "ILL "},		{0xFB, operand_types::impl, "ILL "},
	{0xFC, operand_types::impl, "ILL "},		{0xFD, operand_types::absx, "SBC "},
	{0xFE, operand_types::absx, "INC "},		{0xFF, operand_types::impl, "ILL "}
};

class disassembler {
private:
	word	disassemble_addr;
	bus		*mbus;
public:
	int		last_instruction_size;
	const std::string 	disassemble();
	void	set_address(word address);
	void	set_mainbus(bus *mainbus);
};
