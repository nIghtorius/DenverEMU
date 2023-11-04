#include "cpu2a03_fast.h"
#include <iostream>
#include <cstdint>

#pragma warning(disable : 4996)

cpu2a03_fast::cpu2a03_fast() {
	// initialize cpu.
	strncpy(get_device_descriptor(), "Denver 2a03 CPU (fast emulation)", MAX_DESCRIPTOR_LENGTH);
	regs.pc = 0x0000;
	regs.sp = 0xFD;
	regs.sr = cf_interrupt | cf_break | 0x20;
	regs.x = 0x00;
	regs.y = 0x00;
	regs.ac = 0x00;
	tick_rate = 0x3;
	error_state = false;

	// CPU also has some HW in it.
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;
}

cpu2a03_fast::~cpu2a03_fast() {
}

void cpu2a03_fast::log_register() {
}

void cpu2a03_fast::machine_code_trace(int startaddr, int endaddr, int erraddr) {
	disassembler disasm;
	disasm.set_mainbus(devicebus);
	disasm.set_address(startaddr);
	int	myaddr = startaddr;
	while (myaddr < endaddr) {
		std::string disassembled = disasm.disassemble();
		if (myaddr != erraddr) {
			std::cout << "  0x" << std::hex << (int)myaddr << " " << disassembled << "\n";
		}
		else {
			std::cout << "* 0x" << std::hex << (int)myaddr << " " << disassembled << "\n";
		}
		myaddr += disasm.last_instruction_size;
	}
}

void cpu2a03_fast::write_cpu_log() {
	// logging makes emulation slower, but helpful debugging issues.
	// first we want the disassemble.
	int addr = regs.pc;
	disasm.set_address(addr);
	std::string line_to_exec = disasm.disassemble();

	// now we write
	// <pc> <disassemble> <regs>
	*cpu_log << std::hex << "0x" << (int)addr << " " << line_to_exec << " ";
	*cpu_log << "r.ac=" << (int)regs.ac << ", r.x=" << (int)regs.x << ", r.y="
		<< (int)regs.y << ", r.sp=" << (int)regs.sp << ", r.sr=" << (int)regs.sr << std::endl;
}

void cpu2a03_fast::set_pc(word addr) {
	regs.pc = addr;
}

void cpu2a03_fast::pushstack_byte(byte data) {
	// puts a byte on the stack.
	devicebus->writememory(0x0100 + regs.sp, data);
	regs.sp--;
}

void cpu2a03_fast::pushstack_word(word data) {
	pushstack_byte((data & 0xFF00) >> 8);
	pushstack_byte((data & 0x00FF));
}

byte cpu2a03_fast::pullstack_byte() {
	regs.sp++;
	return devicebus->readmemory(0x0100 + regs.sp);
}

word cpu2a03_fast::pullstack_word() {
	word t;
	t = pullstack_byte();
	t |= (pullstack_byte() << 8);
	return t;
}

int cpu2a03_fast::rundevice(int ticks) {
	int tickcount = 0;
	while (tickcount < ticks) {
		tickcount += rundevice_internal(ticks);
		// dma we need to break immediality. clock module needs this attention.
		if (in_dma_mode) return tickcount;
	}
	return tickcount;
}

int	cpu2a03_fast::rundevice_internal (int ticks) {
	int	actualticks = 0;

	// log state?
	if (cpu_log) write_cpu_log();

	// check dma mode
	if (in_dma_mode) {
		if (dma_cycle == 1) {
			dma_cycle = 0;
			in_dma_mode = false;
			return tick_rate;
		}
		dma_cycle -= 2;
		if (dma_cycle == 0) in_dma_mode = false;
		return 2 * tick_rate;
	}

	// check nmi/irq.
	// NMI
	if (devicebus->nmi_pulled()) {
		nmi_delay = false;
		pushstack_word(regs.pc);
		pushstack_byte(regs.sr & ~0x10);
		regs.sr |= cf_interrupt;
		regs.pc = devicebus->readmemory_as_word(vector_nmi); // NMI vector.
		return 7 * tick_rate;	// NMI takes 7 cycles.
	}
	// IRQ
	if (devicebus->irq_pulled() && ((regs.sr & cf_interrupt) == 0)) {
		pushstack_word(regs.pc);
		pushstack_byte(regs.sr & ~0x10);
		regs.sr |= cf_interrupt;
		regs.pc = devicebus->readmemory_as_word(vector_irq); // IRQ vector.
		irq_delay = false;
		return 7 * tick_rate;	// IRQ takes 7 cycles.
	}

	byte opcode = devicebus->readmemory(regs.pc);
	regs.pc++;
	switch (opcode) {
		case 0x00: {	// BRK instruction
			actualticks += 7;
			regs.pc++;
			pushstack_word(regs.pc);
			pushstack_byte(regs.sr | cf_break | 0x20);
			regs.sr |= cf_interrupt;
			regs.pc = devicebus->readmemory_as_word(vector_irq);	// load brk vector in regs.pc
			break;
		}
		case 0x01: {
			opc_ora(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x02: {
			error_state = true;	// invalid undecodable op.
			break;
		}
		case 0x03: {
			opc_asl(_mem_index_x, true);
			opc_ora(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x04: case 0x44: case 0x64: {
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x05: {
			opc_ora(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x06: {
			opc_asl(_mem_zero, true);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x07: {
			opc_asl(_mem_zero, true);
			opc_ora(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x08: {	// PHP
			actualticks += 3;
			pushstack_byte(regs.sr | 0x30);
			break;
		}
		case 0x09: {
			opc_ora(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x0a: {
			opc_asl(0, false);
			actualticks += 2;
			break;
		}
		case 0x0b: case 0x2b: {// aac?
			opc_and(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			regs.sr = regs.sr & cf_negative ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
			actualticks += 2;
			break;
		}
		case 0x0c: case 0x1c: case 0x3c: case 0x5c: case 0x7c: case 0xdc: case 0xfc: {
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x0d: {
			opc_ora(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x0e: {
			opc_asl(_mem_absolute, true);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x0f: {
			opc_asl(_mem_absolute, true);
			opc_ora(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x10: {
			actualticks += 2;
			if (!(regs.sr & cf_negative)) {
				actualticks++;
				if ((_mem_relative & 0xFF00) != (regs.pc & 0xFF00)) actualticks++;	// boundary tick
				regs.pc = _mem_relative;
			}
			else {
				regs.pc++;
			}
			break;
		}
		case 0x11: {
			opc_ora(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0x12: {
			error_state = true;
			break;
		}
		case 0x13: {
			opc_asl(_mem_index_y, true);
			opc_ora(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 8;
			break;			
		}
		case 0x14: case 0x34: case 0x54: case 0x74: case 0xd4: case 0xf4: {
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x15: {
			opc_ora(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x16: {
			opc_asl(_mem_zero_x, true);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x17: {
			opc_asl(_mem_zero_x, true);
			opc_ora(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x18: {	//clc
			actualticks += 2;
			regs.sr &= 0xFF - cf_carry;
			break;
		}
		case 0x19: {
			opc_ora(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0x1a: {
			error_state = true;
			break;
		}
		case 0x1b: {
			opc_asl(_mem_absolute_y, true);
			opc_ora(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x1d: {
			opc_ora(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0x1e: {
			opc_asl(_mem_absolute_x, true);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x1f: {
			opc_asl(_mem_absolute_x, true);
			opc_ora(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x20: { // jsr
			pushstack_word(regs.pc + 1);
			regs.pc = _mem_absolute;
			actualticks += 6;
			break;
		}
		case 0x21: {
			opc_and(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x22: {
			error_state = true;
			break;
		}
		case 0x23: {
			opc_rol(_mem_index_x, true);
			opc_and(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x24: {
			opc_bit(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x25: {
			opc_and(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x26: {
			opc_rol(_mem_zero, true);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x27: {
			opc_rol(_mem_zero, true);
			opc_and(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x28: { // plp
			actualticks += 4;
			regs.sr = pullstack_byte() | 0x20;
			break;
		}
		case 0x29: {
			opc_and(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x2a: {
			opc_rol(0, false);
			actualticks += 2;
			break;
		}
		case 0x2c: {
			opc_bit(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x2d: {
			opc_and(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x2e: {
			opc_rol(_mem_absolute, true);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x2f: {
			opc_rol(_mem_absolute, true);
			opc_and(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x30: { //bmi
			actualticks += 2;
			if (regs.sr & cf_negative) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative and 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0x31: {
			opc_and(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0x32: {
			error_state = true;
			break;
		}
		case 0x33: {
			opc_rol(_mem_index_y, true);
			opc_and(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x35: {
			opc_and(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x36: {
			opc_rol(_mem_zero_x, true);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x37: {
			opc_rol(_mem_zero_x, true);
			opc_and(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x38: { //sec
			actualticks += 2;
			regs.sr |= cf_carry;
			break;
		}
		case 0x39: {
			opc_and(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0x3a: {
			error_state = true;
			break;
		}
		case 0x3b: {
			opc_rol(_mem_absolute_y, true);
			opc_and(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x3d: {
			opc_and(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0x3e: {
			opc_rol(_mem_absolute_x, true);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x3f: {
			opc_rol(_mem_absolute_x, true);
			opc_and(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x40: { // rti
			actualticks += 6;
			regs.sr = pullstack_byte() | 0x20;
			regs.pc = pullstack_word();
			break;
		}
		case 0x41: {
			opc_xor(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x42: {
			error_state = true;
			break;
		}
		case 0x43: {
			opc_lsr(_mem_index_x, true);
			opc_xor(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x45: {
			opc_xor(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x46: {
			opc_lsr(_mem_zero, true);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x47: {
			opc_lsr(_mem_zero, true);
			opc_xor(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x48: { // pha
			actualticks += 3;
			pushstack_byte(regs.ac);
			break;
		}
		case 0x49: {
			opc_xor(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x4a: {
			opc_lsr(0, false);
			actualticks += 2;
			break;
		}
		case 0x4b: { // alr
			opc_and(devicebus->readmemory(_mem_immediate));
			opc_lsr(0, false);
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x4c: {	//jmp
			regs.pc = _mem_absolute;
			actualticks += 3;
			break;
		}
		case 0x4d: {
			opc_xor(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x4e: {
			opc_lsr(_mem_absolute, true);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x4f: {
			opc_lsr(_mem_absolute, true);
			opc_xor(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x50: {
			actualticks += 2;
			if (!(regs.sr & cf_overflow)) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0x51: {
			opc_xor(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0x52: {
			error_state = true;
			break;
		}
		case 0x53: {
			opc_lsr(_mem_index_y, true);
			opc_xor(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x55: {
			opc_xor(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x56: {
			opc_lsr(_mem_zero_x, true);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x57: {
			opc_lsr(_mem_zero_x, true);
			opc_xor(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x58: { // cli
			actualticks += 2;
			regs.sr &= 0xFF - cf_interrupt;
			break;
		}
		case 0x59: {
			opc_xor(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0x5a: {
			error_state = true;
			break;
		}
		case 0x5b: {
			opc_lsr(_mem_absolute_y, true);
			opc_xor(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x5d: {
			opc_xor(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0x5e: {
			opc_lsr(_mem_absolute_x, true);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x5f: {
			opc_lsr(_mem_absolute_x, true);
			opc_xor(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x60: { // rts
			actualticks += 6;
			regs.pc = pullstack_word();
			regs.pc++;
			break;
		}
		case 0x61: {
			opc_adc(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x62: {
			error_state = true;
			break;
		}
		case 0x63: {
			opc_ror(_mem_index_x, true);
			opc_adc(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x65: {
			opc_adc(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x66: {
			opc_ror(_mem_zero, true);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x67: {
			opc_ror(_mem_zero, true);
			opc_adc(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0x68: { // pla
			actualticks += 4;
			regs.ac = pullstack_byte();
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
			regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
			break;
		}
		case 0x69: {
			opc_adc(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x6a: {
			opc_ror(0, false);
			actualticks += 2;
			break;
		}
		case 0x6b: { //arr  (have no idea how it actually works. but ac & imm, ror (ac) general)
			byte data = devicebus->readmemory(_mem_immediate);
			data &= regs.ac;
			regs.ac = (data >> 1) | ((regs.sr & cf_carry) << 7);
			regs.sr &= 0xFF - (cf_zero | cf_negative);
			regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
			regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = regs.ac & 0x40 ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
			regs.sr = (regs.ac ^ (regs.ac << 1)) & 0x40 ? regs.sr | cf_overflow : regs.sr & (0xFF - cf_overflow);
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x6c: {
			regs.pc = _mem_indexed;
			actualticks += 5;
			break;
		}
		case 0x6d: {
			opc_adc(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x6e: {
			opc_ror(_mem_absolute, true);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x6f: {
			opc_ror(_mem_absolute, true);
			opc_adc(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0x70: { // bvs
			actualticks += 2;
			if (regs.sr & cf_overflow) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0x71: {
			opc_adc(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0x72: {
			error_state = true;
			break;
		}
		case 0x73: {
			opc_ror(_mem_index_y, true);
			opc_adc(devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0x75: {
			opc_adc(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x76: {
			opc_ror(_mem_zero_x, true);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x77: {
			opc_ror(_mem_zero_x, true);
			opc_adc(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x78: { // sei
			actualticks += 2;
			regs.sr |= cf_interrupt;
			break;
		}
		case 0x79: {
			opc_adc(devicebus->readmemory(_mem_absolute_y));
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			regs.pc += 2;
			break;
		}
		case 0x7a: {
			error_state = true;
			break;
		}
		case 0x7b: {
			opc_ror(_mem_absolute_y, true);
			opc_adc(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x7d: {
			opc_adc(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0x7e: {
			opc_ror(_mem_absolute_x, true);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x7f: {
			opc_ror(_mem_absolute_x, true);
			opc_adc(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0x80: case 0x82: case 0x89: case 0xc2: case 0xe2: {
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0x81: {
			devicebus->writememory(_mem_index_x, regs.ac);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x83: {
			devicebus->writememory(_mem_index_x, regs.ac & regs.x);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x84: {
			devicebus->writememory(_mem_zero, regs.y);
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x85: {
			devicebus->writememory(_mem_zero, regs.ac);
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x86: {
			devicebus->writememory(_mem_zero, regs.x);
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x87: {
			devicebus->writememory(_mem_zero, regs.ac & regs.x);
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0x88: { // dey
			regs.y = opc_dec(regs.y);
			actualticks += 2;
			break;
		}
		case 0x8a: { // txa
			actualticks += 2;
			regs.ac = regs.x;
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
			break;
		}
		case 0x8b: {
			error_state = true;
			break;
		}
		case 0x8c: {
			devicebus->writememory(_mem_absolute, regs.y);
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x8d: {
			devicebus->writememory(_mem_absolute, regs.ac);
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x8e: {
			devicebus->writememory(_mem_absolute, regs.x);
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x8f: {
			devicebus->writememory(_mem_absolute, regs.ac & regs.x);
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0x90: { // bcc
			actualticks += 2;
			if (!(regs.sr & cf_carry)) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;				
			} else regs.pc++;
			break;
		}
		case 0x91: {
			devicebus->writememory(_mem_index_y, regs.ac);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0x92: case 0x93: {
			error_state = true;
			break;
		}
		case 0x94: {
			devicebus->writememory(_mem_zero_x, regs.y);
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x95: {
			devicebus->writememory(_mem_zero_x, regs.ac);
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x96: {
			devicebus->writememory(_mem_zero_y, regs.x);
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x97: {
			devicebus->writememory(_mem_zero_y, regs.ac & regs.x);
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0x98: { // tya
			actualticks += 2;
			regs.ac = regs.y;
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
			break;
		}
		case 0x99: {
			devicebus->writememory(_mem_absolute_y, regs.ac);
			regs.pc += 2;
			actualticks += 5;
			break;

		}
		case 0x9a: {	// txs
			actualticks += 2;
			regs.sp = regs.x;
			break;
		}
		case 0x9b: {
			error_state = true;
			break;
		}
		case 0x9c: {	// keeps failing 07-abs_xy.nes test, everything documented tells me this is right.
			byte hb = (_mem_absolute >> 8);
			devicebus->writememory(_mem_absolute_x, regs.y & (hb + 1));
			regs.pc += 2;
			actualticks += 5;
			break;
		}
		case 0x9d: {
			devicebus->writememory(_mem_absolute_x, regs.ac);
			regs.pc += 2;
			actualticks += 5;
			break;
		}
		case 0x9e: {	// keeps failing 07 - abs_xy.nes test, everything documented tells me this is right.
			byte hb = (_mem_absolute >> 8);
			devicebus->writememory(_mem_absolute_y, regs.x & (hb + 1));
			regs.pc += 2;
			actualticks += 5;
			break;
		}
		case 0x9f:  {
			error_state = true;
			break;
		}
		case 0xa0: {
			opc_ld_(regs.y, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xa1: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xa2: {
			opc_ld_(regs.x, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xa3: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_index_x));
			regs.x = regs.ac;
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xa4: {
			opc_ld_(regs.y, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xa5: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xa6: {
			opc_ld_(regs.x, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xa7: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_zero));
			regs.x = regs.ac;
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xa8: { // tay
			actualticks += 2;
			regs.y = regs.ac;
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = regs.y & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !regs.y ? regs.sr | cf_zero : regs.sr;
			break;
		}
		case 0xa9: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xaa: { // tax
			actualticks += 2;
			regs.x = regs.ac;
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = regs.x & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !regs.x ? regs.sr | cf_zero : regs.sr;
			break;
		}
		case 0xab: { // atx
			opc_ld_(regs.ac, devicebus->readmemory(_mem_immediate));
			regs.x = regs.ac;
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xac: {
			opc_ld_(regs.y, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xad: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xae: {
			opc_ld_(regs.x, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xaf: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_absolute));
			regs.x = regs.ac;
			regs.pc += 2;
			actualticks += 4;
			break;			
		}
		case 0xb0: { //bcs
			actualticks += 2;
			if (regs.sr & cf_carry) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0xb1: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0xb2: {
			error_state = true;
			break;
		}
		case 0xb3: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_index_y));
			regs.pc++;
			regs.x = regs.ac;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0xb4: {
			opc_ld_(regs.y, devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xb5: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0xb6: {
			opc_ld_(regs.x, devicebus->readmemory(_mem_zero_y));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0xb7: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_zero_y));
			regs.x = regs.ac;
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0xb8: { // clv
			regs.sr &= 0xFF - cf_overflow;
			actualticks += 2;
			break;
		}
		case 0xb9: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0xba: { //tsx
			actualticks += 2;
			regs.x = regs.sp;
			regs.sr &= 0xFF - (cf_negative | cf_zero);
			regs.sr = regs.x & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !regs.x ? regs.sr | cf_zero : regs.sr;
			break;
		}
		case 0xbb: {
			error_state = true;
			break;
		}
		case 0xbc: {
			opc_ld_(regs.y, devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0xbd: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0xbe: {
			opc_ld_(regs.x, devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0xbf: {
			opc_ld_(regs.ac, devicebus->readmemory(_mem_absolute_y));
			regs.x = regs.ac;
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0xc0: {
			opc_cmp(regs.y, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xc1: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_index_x));
			actualticks += 6;
			regs.pc++;
			break;
		}
		case 0xc3: {
			byte t = opc_dec(devicebus->readmemory(_mem_index_x));
			devicebus->writememory(_mem_index_x, t);
			opc_cmp(regs.ac, t);
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0xc4: {
			opc_cmp(regs.y, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xc5: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xc6: {
			devicebus->writememory(_mem_zero, opc_dec(devicebus->readmemory(_mem_zero)));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0xc7: {
			byte t = opc_dec(devicebus->readmemory(_mem_zero));
			devicebus->writememory(_mem_zero, t);
			opc_cmp(regs.ac, t);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0xc8: { //iny
			regs.y = opc_inc(regs.y);
			actualticks += 2;
			break;
		}
		case 0xc9: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xca: { //dex
			regs.x = opc_dec(regs.x);
			actualticks += 2;
			break;
		}
		case 0xcb: { //axs
			// what does it do.
			// X <- (A & X) - Immediate
			byte data = devicebus->readmemory(_mem_immediate);
			regs.sr &= 0xFF - (cf_negative | cf_zero | cf_carry);
			regs.x &= regs.ac;
			word c = word(regs.x) - data;
			regs.sr = regs.x >= data ? regs.sr | cf_carry : regs.sr;
			regs.sr = c & 0x80 ? regs.sr | cf_negative : regs.sr;
			regs.sr = !(c & 0xFF) ? regs.sr | cf_zero : regs.sr;
			regs.x = byte(c);
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xcc: {
			opc_cmp(regs.y, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xcd: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xce: {
			devicebus->writememory(_mem_absolute, opc_dec(devicebus->readmemory(_mem_absolute)));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0xcf: {
			byte t = opc_dec(devicebus->readmemory(_mem_absolute));
			devicebus->writememory(_mem_absolute, t);
			opc_cmp(regs.ac, t);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0xd0: { // bne
			actualticks += 2;
			if (!(regs.sr & cf_zero)) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0xd1: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_index_y));
			regs.pc++;
			actualticks += 5;
			if (_mem_index_y_inc) actualticks++;
			break;
		}
		case 0xd2: {
			error_state = true;
			break;
		}
		case 0xd3: {
			byte t = opc_dec(devicebus->readmemory(_mem_index_y));
			devicebus->writememory(_mem_index_y, t);
			opc_cmp(regs.ac, t);
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0xd5: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0xd6: {
			devicebus->writememory(_mem_zero_x, opc_dec(devicebus->readmemory(_mem_zero_x)));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xd7: {
			byte t = opc_dec(devicebus->readmemory(_mem_zero_x));
			devicebus->writememory(_mem_zero_x, t);
			opc_cmp(regs.ac, t);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xd8: { //cld
			regs.sr &= 0xff - cf_decimal;
			actualticks += 2;
			break;
		}
		case 0xd9: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0xda: {
			error_state = true;
			break;
		}
		case 0xdb: {
			byte t = opc_dec(devicebus->readmemory(_mem_absolute_y));
			devicebus->writememory(_mem_absolute_y, t);
			opc_cmp(regs.ac, t);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0xdd: {
			opc_cmp(regs.ac, devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0xde: {
			devicebus->writememory(_mem_absolute_x, opc_dec(devicebus->readmemory(_mem_absolute_x)));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0xdf: {
			byte t = opc_dec(devicebus->readmemory(_mem_absolute_x));
			devicebus->writememory(_mem_absolute_x, t);
			opc_cmp(regs.ac, t);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0xe0: {
			opc_cmp(regs.x, devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xe1: {
			opc_sbc(devicebus->readmemory(_mem_index_x));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xe3: {
			byte t = opc_inc(devicebus->readmemory(_mem_index_x));
			devicebus->writememory(_mem_index_x, t);
			opc_sbc(t);
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0xe4: {
			opc_cmp(regs.x, devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xe5: {
			opc_sbc(devicebus->readmemory(_mem_zero));
			regs.pc++;
			actualticks += 3;
			break;
		}
		case 0xe6: {
			devicebus->writememory(_mem_zero, opc_inc(devicebus->readmemory(_mem_zero)));
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0xe7: {
			byte t = opc_inc(devicebus->readmemory(_mem_zero));
			devicebus->writememory(_mem_zero, t);
			opc_sbc(t);
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0xe8: { // inx
			regs.x = opc_inc(regs.x);
			actualticks += 2;
			break;
		}
		case 0xe9: case 0xeb: {
			opc_sbc(devicebus->readmemory(_mem_immediate));
			regs.pc++;
			actualticks += 2;
			break;
		}
		case 0xea: { // nop
			actualticks += 2;
			break;
		}
		case 0xec: {
			opc_cmp(regs.x, devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xed: {
			opc_sbc(devicebus->readmemory(_mem_absolute));
			regs.pc += 2;
			actualticks += 4;
			break;
		}
		case 0xee: {
			devicebus->writememory(_mem_absolute, opc_inc(devicebus->readmemory(_mem_absolute)));
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0xef: {
			byte t = opc_inc(devicebus->readmemory(_mem_absolute));
			devicebus->writememory(_mem_absolute, t);
			opc_sbc(t);
			regs.pc += 2;
			actualticks += 6;
			break;
		}
		case 0xf0: { // beq
			actualticks += 2;
			if (regs.sr & cf_zero) {
				actualticks++;
				if ((regs.pc & 0xFF00) != (_mem_relative & 0xFF00)) actualticks++;
				regs.pc = _mem_relative;
			} else regs.pc++;
			break;
		}
		case 0xf1: {
			opc_sbc(devicebus->readmemory(_mem_index_y));
			if (_mem_index_y_inc) actualticks++;
			regs.pc++;
			actualticks += 5;
			break;
		}
		case 0xf2: {
			error_state = true;
			break;
		}
		case 0xf3: {
			byte t = opc_inc(devicebus->readmemory(_mem_index_y));
			devicebus->writememory(_mem_index_y, t);
			opc_sbc(t);
			regs.pc++;
			actualticks += 8;
			break;
		}
		case 0xf5: {
			opc_sbc(devicebus->readmemory(_mem_zero_x));
			regs.pc++;
			actualticks += 4;
			break;
		}
		case 0xf6: {
			devicebus->writememory(_mem_zero_x, opc_inc(devicebus->readmemory(_mem_zero_x)));
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xf7: {
			byte t = opc_inc(devicebus->readmemory(_mem_zero_x));
			devicebus->writememory(_mem_zero_x, t);
			opc_sbc(t);
			regs.pc++;
			actualticks += 6;
			break;
		}
		case 0xf8: { //sed
			regs.sr |= cf_decimal;
			actualticks += 2;
			break;
		}
		case 0xf9: {
			opc_sbc(devicebus->readmemory(_mem_absolute_y));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_y & 0xFF00)) actualticks++;
			break;
		}
		case 0xfa: {
			error_state = true;
			break;
		}
		case 0xfb: {
			byte t = opc_inc(devicebus->readmemory(_mem_absolute_y));
			devicebus->writememory(_mem_absolute_y, t);
			opc_sbc(t);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0xfd: {
			opc_sbc(devicebus->readmemory(_mem_absolute_x));
			regs.pc += 2;
			actualticks += 4;
			if ((_mem_absolute & 0xFF00) != (_mem_absolute_x & 0xFF00)) actualticks++;
			break;
		}
		case 0xfe: {
			devicebus->writememory(_mem_absolute_x, opc_inc(devicebus->readmemory(_mem_absolute_x)));
			regs.pc += 2;
			actualticks += 7;
			break;
		}
		case 0xff: {
			byte t = opc_inc(devicebus->readmemory(_mem_absolute_x));
			devicebus->writememory(_mem_absolute_x, t);
			opc_sbc(t);
			regs.pc += 2;
			actualticks += 7;
			break;
		}
	}
	return actualticks * tick_rate;
}
/*
	opcode helpers
*/

void cpu2a03_fast::opc_ld_(byte &rdata, byte data) {
	rdata = data;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !data ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_cmp(byte rdata, byte data) {
	regs.sr &= 0xFF - (cf_negative | cf_zero | cf_carry);
	word c = word(rdata) - data;
	regs.sr = rdata >= data ? regs.sr | cf_carry : regs.sr;
	regs.sr = c & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !(c & 0xFF) ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_ora(byte data) {
	regs.ac |= data;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_asl(word addr, bool memory) {
	byte data = memory ? devicebus->readmemory(addr) : regs.ac;
	regs.sr = data & 0x80 ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
	data = data << 1;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !data ? regs.sr | cf_zero : regs.sr;
	if (memory) devicebus->writememory(addr, data); else regs.ac = data;
}

void cpu2a03_fast::opc_and(byte data) {
	regs.ac &= data;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_rol(word addr, bool memory) {
	bool carry_bit = (regs.sr & cf_carry);
	byte data = memory ? devicebus->readmemory(addr) : regs.ac;
	regs.sr = data & 0x80 ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
	data = data << 1;
	if (carry_bit) data++;	// add carry
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !data ? regs.sr | cf_zero : regs.sr;
	if (memory) devicebus->writememory(addr, data); else regs.ac = data;
}

void cpu2a03_fast::opc_ror(word addr, bool memory) {
	bool carry_bit = (regs.sr & cf_carry);
	byte data = memory ? devicebus->readmemory(addr) : regs.ac;
	regs.sr = data & 0x01 ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
	data = data >> 1;
	if (carry_bit) data += 0x80;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !data ? regs.sr | cf_zero : regs.sr;
	if (memory) devicebus->writememory(addr, data); else regs.ac = data;
}

void cpu2a03_fast::opc_bit(byte data) {
	regs.sr &= 0xFF - (cf_negative | cf_zero | cf_overflow);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = data & 0x40 ? regs.sr | cf_overflow : regs.sr;
	regs.sr = !(regs.ac & data) ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_xor(byte data) {
	regs.ac ^= data;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = regs.ac & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !regs.ac ? regs.sr | cf_zero : regs.sr;
}

void cpu2a03_fast::opc_lsr(word addr, bool memory) {
	byte data = memory ? devicebus->readmemory(addr) : regs.ac;
	regs.sr = data & 0x01 ? regs.sr | cf_carry : regs.sr & (0xFF - cf_carry);
	data = data >> 1;
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	regs.sr = data & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !data ? regs.sr | cf_zero : regs.sr;
	if (memory) devicebus->writememory(addr, data); else regs.ac = data;
}

void cpu2a03_fast::opc_adc(byte data) {
	bool carry_bit = (regs.sr & cf_carry);
	word data2 = carry_bit ? regs.ac + 1 : regs.ac;
	data2 += data;
	regs.sr &= 0xFF - (cf_overflow | cf_carry | cf_negative | cf_zero);
	regs.sr = ((regs.ac ^ byte(data2)) & ((data ^ byte(data2)) & 0x80)) ? regs.sr | cf_overflow : regs.sr;
	regs.sr = data2 > 0xFF ? regs.sr | cf_carry : regs.sr;
	regs.sr = data2 & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !(data2 & 0xFF) ? regs.sr | cf_zero : regs.sr;
	regs.ac = byte(data2);
}

void cpu2a03_fast::opc_sbc(byte data) {
	opc_adc(data ^ 0xFF);		// that was easy
}

byte cpu2a03_fast::opc_dec(byte data) {
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	byte t = data - 1;
	regs.sr = t & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !t ? regs.sr | cf_zero : regs.sr;
	return t;
}

byte cpu2a03_fast::opc_inc(byte data) {
	regs.sr &= 0xFF - (cf_negative | cf_zero);
	byte t = data + 1;
	regs.sr = t & 0x80 ? regs.sr | cf_negative : regs.sr;
	regs.sr = !t ? regs.sr | cf_zero : regs.sr;
	return t;
}

/*
	General stuff
*/

void cpu2a03_fast::reset() {
	regs.pc = devicebus->readmemory_as_word(vector_reset);		// load reset vector @ 0xFFFC
	regs.sp = 0xFD;
	regs.sr |= cf_interrupt;
	error_state = false;
}

void cpu2a03_fast::coldboot() {
	if (!devicebus) {
		throw new std::runtime_error("cpu2a03_fast::coldboot() called without defined memory bus!\r\nPlease assign memory bus before calling coldboot()");
		return;
	}

	devicebus->busreset();

	regs.pc = devicebus->readmemory_as_word(vector_reset);
	regs.sp = 0xFD;
	regs.sr = cf_interrupt | cf_break | 0x20;
	regs.sr = 0x24;
	regs.x = 0x00;
	regs.y = 0x00;
	regs.ac = 0x00;
	in_dma_mode = false;
	
	error_state = false;
}

void cpu2a03_fast::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base == 0x14) {
		// DMA request.
		in_dma_mode = true;	// chip is outputting DMA
		dma_cycle = 512;		// 512 cpu ticks = 1536 ppu ticks. (*3)
		//if (ticksdone % 2) dma_cycle++;
		dma_count = 255;
		dma_high = data;
		return; // done processing.
	}
}

void cpu2a03_fast::dma(byte *data, bool is_output, bool started) {
	if (!is_output) {
		//std::cout << "dma(" << std::dec << (int)dma_count << ") -- addr: " << std::hex << "0x" << (int)(dma_high << 8 | (255 - dma_count)); 
		*data = devicebus->readmemory(dma_high << 8 | (255 - dma_count));
		//std::cout << ", read byte : 0x" << (int)*data << std::dec << " | dma_cycle: " << dma_cycle << std::endl;
		dma_count--;
	}
}

void cpu2a03_fast::write_execution_log() {
	if (cpu_log) return;
	cpu_log = new std::ofstream("cpu_exec_log.txt", std::ios::out);
	*cpu_log << "Denver CPU log\n\n";
	disasm.set_mainbus(devicebus);
}

void cpu2a03_fast::stop_execution_log() {
	if (!cpu_log) return;
	cpu_log->close();
	delete cpu_log;
	cpu_log = nullptr;
}