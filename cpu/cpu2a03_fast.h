/*

	cpu emulation core
	cpu emulation type: fast

*/
#pragma once
#include "../bus/bus.h"
#include <fstream>
#include "tools/2a03_disasm.h"

// memory handling MACROs
#define	_mem_immediate		regs.pc
#define	_mem_zero			devicebus->readmemory (regs.pc)
#define _mem_zero_x			(devicebus->readmemory (regs.pc) + regs.x) & 0xFF
#define _mem_zero_y			(devicebus->readmemory (regs.pc) + regs.y) & 0xFF
#define _mem_absolute		devicebus->readmemory_as_word (regs.pc)
#define _mem_absolute_x		word(devicebus->readmemory_as_word (regs.pc) + regs.x)
#define _mem_absolute_y		word(devicebus->readmemory_as_word (regs.pc) + regs.y)
#define _mem_index_x		devicebus->readmemory_as_word_wrap ((devicebus->readmemory (regs.pc) + regs.x) & 0xFF)
#define _mem_index_y		word(devicebus->readmemory_as_word_wrap ((devicebus->readmemory (regs.pc))) + regs.y)
#define _mem_index_y_inc	(devicebus->readmemory_as_word_wrap (devicebus->readmemory (regs.pc)) & 0xFF00) != ((devicebus->readmemory_as_word_wrap ((devicebus->readmemory (regs.pc))) + regs.y) & 0xFF00)
#define _mem_relative		(!(devicebus->readmemory (regs.pc) & 0x80)) ? regs.pc + devicebus->readmemory (regs.pc) + 1: regs.pc - ((128 - word(devicebus->readmemory (regs.pc)) - 1) & 0x7F)
#define _mem_indexed		devicebus->readmemory_as_word_wrap (devicebus->readmemory_as_word(regs.pc))

// processor flags
#define cf_negative			0x80
#define cf_overflow			0x40
#define cf_break			0x10
#define cf_decimal			0x08
#define cf_interrupt		0x04
#define cf_zero				0x02
#define cf_carry			0x01

// vectors.
#define vector_nmi			0xFFFA
#define vector_reset		0xFFFC
#define vector_irq			0xFFFE

struct cpuregs {
	word		pc;
	byte		ac;
	byte		x;
	byte		y;
	byte		sp;
	byte		sr;
};

class cpu2a03_fast: public bus_device {
private:
	// opcode helpers.
	void	opc_ora(byte data);
	void	opc_asl(word addr, bool memory);
	void	opc_and(byte data);
	void	opc_rol(word addr, bool memory);
	void	opc_ror(word addr, bool memory);
	void	opc_bit(byte data);
	void	opc_xor(byte data);
	void	opc_lsr(word addr, bool memory);
	void	opc_adc(byte data);
	void	opc_sbc(byte data);
	byte	opc_dec(byte data);
	byte	opc_inc(byte data);
	void	opc_ld_(byte &rdata, byte data);
	void	opc_cmp(byte rdata, byte data);
	int		rundevice_internal(int ticks);
	void	write_cpu_log();

	int		dma_cycle = 0;
	byte	dma_high = 0;
	byte	dma_count = 0;
	bool	nmi_delay = 0;
	bool	irq_delay = 0;

	std::ofstream	*cpu_log = nullptr;
	disassembler	disasm;

public:	
	cpuregs	regs;
	cpuregs lastregs;

	bool	error_state = false;
	byte	last_opcode = 0x00;

	cpu2a03_fast();
	~cpu2a03_fast();
	int		rundevice(int ticks);
	int		oc_rundevice(int ticks);	// run the CPU but report lower ticks. essentially OC'ing the cpu. trying this for 120hz NSFs
	void	reset();
	void	coldboot();

	//stack
	void	pushstack_word(word data);
	void	pushstack_byte(byte data);
	byte	pullstack_byte();
	word	pullstack_word();

	// memory bus
	void	write(int addr, int addr_from_base, byte data);

	// dma
	void	dma(byte *data, bool is_output, bool started);

	//debug
	void	log_register();
	void	set_pc(word addr);
	void	write_execution_log();
	void	stop_execution_log();
	void	machine_code_trace(int startaddr, int endaddr, int erraddr);
	void	stack_dump();
	bool	stack_overflow();
};