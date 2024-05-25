/*

	disasm implementation.

*/

#include "2a03_disasm.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

const std::string		disassembler::disassemble() {
	if (!mbus) return nullptr;

	word before = disassemble_addr;

	byte opcb = mbus->readmemory(disassemble_addr++, true);
	opcode opc = opcode_table[opcb];

	std::stringstream dis_str;
	dis_str << opc.mnemonic << std::setw(0x08) << std::setfill(' ') << std::hex;

	switch (opc.optype) {
	case operand_types::acc:
		dis_str << " A";
		break;
	case operand_types::absolute:
		dis_str << " 0x" << std::setw(4) << std::setfill('0') << (uint16_t)mbus->readmemory_as_word(disassemble_addr, true);
		disassemble_addr += 2;
		break;
	case operand_types::absx:
		dis_str << " 0x" << std::setw(4) << std::setfill('0') << (uint16_t)mbus->readmemory_as_word(disassemble_addr, true) << ", X";
		disassemble_addr += 2;
		break;
	case operand_types::absy:
		dis_str << " 0x" << std::setw(4) << std::setfill('0') << (uint16_t)mbus->readmemory_as_word(disassemble_addr, true) << ", Y";
		disassemble_addr += 2;
		break;
	case operand_types::imm:
		dis_str << "  #0x" << std::setw(2) << std::setfill('0') << (int)mbus->readmemory(disassemble_addr, true) << std::setw(4);
		disassemble_addr++;
		break;
	case operand_types::impl:
		break;
	case operand_types::ind:
		dis_str << " (0x" << std::setw(4) << std::setfill('0') << (uint16_t)mbus->readmemory_as_word(disassemble_addr, true) << ")";
		disassemble_addr += 2;
		break;
	case operand_types::xind:
		dis_str << " (0x" << std::setw(4) << std::setfill('0') << (uint16_t)mbus->readmemory(disassemble_addr, true) << ", X)";
		disassemble_addr++;
		break;
	case operand_types::indy:
		dis_str << " (0x" << std::setw(2) << std::setfill('0') << (uint16_t)mbus->readmemory(disassemble_addr, true) << "), Y";
		disassemble_addr++;
		break;
	case operand_types::rel:
		dis_str << " 0x" << std::setw(4) << std::setfill('0') << (int)((uint16_t)disassemble_addr + 1 + (int8_t)mbus->readmemory(disassemble_addr, true));
		disassemble_addr++;
		break;
	case operand_types::zpg:
		dis_str << " 0x" << std::setw(2) << std::setfill('0') << (uint16_t)mbus->readmemory(disassemble_addr, true);
		disassemble_addr++;
		break;
	case operand_types::zpgx:
		dis_str << " 0x" << std::setw(2) << std::setfill('0') << (uint16_t)mbus->readmemory(disassemble_addr, true) << ", X";
		disassemble_addr++;
		break;
	case operand_types::zpgy:
		dis_str << " 0x" << std::setw(2) << std::setfill('0') << (uint16_t)mbus->readmemory(disassemble_addr, true) << ", Y";
		disassemble_addr++;
		break;
	}
	
	last_instruction_size = disassemble_addr - before;

	return dis_str.str();
}

void	disassembler::set_address(word address) {
	disassemble_addr = address;
}

void	disassembler::set_mainbus(bus *mainbus) {
	mbus = mainbus;
}