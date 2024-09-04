/*

	implementation of the vrc2x,vrc4x rom mapper.
	first NES 2.0 compatible mapper for Denver.

*/

#pragma warning(disable : 4996)

#include "mapper_021-22-23-25.h"
#include <iostream>

word	vrc2_4_rom::recompute_addr(word addr) {
	// recompute address by eval'ing vrc2/4 & submapper.

	// do we have compat mode enabled?
	switch (compability_mode) {
	case VRC24_COMPAT_MAPPER_21:
		// we are doing VRC4a or VRC4c possibly.
		// 4a = A1, A2, 4c = A6, A7
		if ((addr & 0x02) || (addr & 0x04)) {
			std::cout << "VRC2/4 Probable chip detected: VRC4a\n";
			submapper = 1;	// VRC4a detected.
			run_as_mapper = 21;
			compability_mode = -1;		// compat detect done.
		}
		if ((addr & 0x80) || (addr & 0x40)) {
			std::cout << "VRC2/4 Probable chip detected: VRC4c\n";
			submapper = 2;	// VRC4c detected.
			run_as_mapper = 21;
			compability_mode = -1;		// compat detect done.
		}
		state.ram_enable = true;	// force ram enable in compat
		break;
	case VRC24_COMPAT_MAPPER_23:
		// we are doing VRC2b+VRC4f, VRC4e
		// 2b/4f = A0, A1, 4e = A2, A3
		if ((addr & 0x01) || (addr & 0x02)) {
			std::cout << "VRC2/4 Probable chip detected: VRC2b/VRC4f\n";
			submapper = 1;	// emulate VRC4f
			run_as_mapper = 23;
			compability_mode = -1;		// compat detect done.
		}
		if ((addr & 0x04) || (addr & 0x08)) {
			std::cout << "VRC2/4 Probable chip detected: VRC4e\n";
			submapper = 2;	// emulate VRC4e
			run_as_mapper = 23; 
			compability_mode = -1;		// compat detect done.
		}
		state.ram_enable = true;	// force ram enable in compat
		break;
	case VRC24_COMPAT_MAPPER_25:
		// we are doing VRC2c+VRC4b, VRC4d
		// 2c/4b = A1, A0, 4d = A3, A2
		if ((addr & 0x01) || (addr & 0x02)) {
			std::cout << "VRC2/4 Probable chip detected: VRC2c/VRC4b\n";
			submapper = 1; // emulate VRC4b
			run_as_mapper = 25;
			compability_mode = -1;		// compat detect done.
		}
		if ((addr & 0x04) || (addr & 0x08)) {
			std::cout << "VRC2/4 Probable chip detected: VRC4d\n";
			submapper = 2; // emulate VRC4d
			run_as_mapper = 25; 
			compability_mode = -1;		// compat detect done.
		}
		state.ram_enable = true;	// force ram enable in compat
		break;
	}

	byte a0, a1;
	switch (run_as_mapper) {
	case 21:
		switch (submapper) {
		case 0:
		case 1:
			// vrc4a (ines1.0/2.0)
			a0 = addr & 2;		// A1
			a1 = addr & 4;		// A2
			return ((addr & ~0x07) | (a0 >> 1) | (a1 >> 1));
			break;
		case 2:
			// vrc4c (nes2.0 only)
			a0 = addr & 64;		// A6
			a1 = addr & 128;	// A7
			return ((addr & ~0xFF) | (a0 >> 6) | (a1 >> 6));
			break;
		}
		break;
	case 22:
		switch (submapper) {
		case 0:
			// vrc2a (ines simple)
			a0 = addr & 2;
			a1 = addr & 1;
			return ((addr & ~0x03) | (a1 << 1) | (a0 >> 1));
			break;
		}
		break;
	case 23:
		switch (submapper) {
		case 0:
		case 1:
			// vrc4f
			return addr;		// as-is
			break;
		case 2:
			// vrc4e
			a0 = addr & 4;		// A2
			a1 = addr & 8;		// A3
			return ((addr & ~0xFF) | (a0 >> 2) | (a1 >> 2));
			break;
		case 3:
			// vrc2b (wut?)
			return addr;		// as-is
			break;
		}
		break;
	case 25:
		switch (submapper) {
		case 0:
		case 1:
			// vrc4b
			a0 = addr & 2;	// A1
			a1 = addr & 1;	// A0
			return ((addr & ~0x03) | (a1 << 1) | (a0 >> 1));
			break;
		case 2:
			// vrc4d
			a0 = addr & 8;	// A3
			a1 = addr & 4;	// A2
			return ((addr & ~0xFF) | (a0 >> 3) | (a1 >> 1));
			break;
		case 3:
			// vrc2c (wut? again?)
			a0 = addr & 2;	// A1
			a1 = addr & 1;	// A0
			return ((addr & ~0x03) | (a1 << 1) | (a0 >> 1));
		}
		break;
	}
	return addr; // just return addr as-is if no condition is met.
}

vrc2_4_rom::vrc2_4_rom() {
	strncpy(get_device_descriptor(), "Denver VRC2/4 Mapper ROM (Multiple)", MAX_DESCRIPTOR_LENGTH);
	ram = (byte*)malloc(8192); // 8K of RAM.
	memset(&state, 0, sizeof(vrc2_4_state));
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	set_debug_data();
}

vrc2_4_rom::~vrc2_4_rom() {
	free(ram);
}

batterybackedram* vrc2_4_rom::get_battery_backed_ram() {
	return new batterybackedram((byte*)ram, 8192);
}

void vrc2_4_rom::set_battery_backed_ram(byte* data, std::size_t size) {
	if (size > 8192) return;
	memcpy(ram, data, size);
}


int		vrc2_4_rom::rundevice(int ticks) {
	if (!state.irq_enabled) return ticks;
	if (vrc2_mode) return ticks; // vrc2(x) does not have an IRQ counter.

	if (state.irq_mode == 0) {
		for (int i = 0; i < ticks; i++) {
			state.prescaler -= 3;
			if (state.prescaler <= 0) {
				state.prescaler += 341;
				if (state.irq_latch == 0xFF) {
					irq_enable = true;	// raise IRQ.		
					state.irq_latch = state.irq_latch_reload;
				}
				else {
					state.irq_latch++;
				}
			}
		}
	}
	if (state.irq_mode == 1) {
		for (int i = 0; i < ticks; i++) {
			if (state.irq_latch == 0xFF) {
				irq_enable = true;
				state.irq_latch = state.irq_latch_reload;
			}
			else {
				state.irq_latch++;
			}
		}
	}

	return ticks;
}

byte	vrc2_4_rom::read(int addr, int addr_from_base, bool onlyread) {
	// PRG RAM (VRC2 always enabled, VRC4 can be disabled)
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (vrc2_mode) return ram[addr - 0x6000];
		if (state.ram_enable) return ram[addr - 0x6000];
		return BUS_OPEN_BUS;
	}
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) return prg_8000[addr - 0x8000];
	if ((addr >= 0xA000) && (addr <= 0xBFFF)) return prg_a000[addr - 0xa000];
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) return prg_c000[addr - 0xc000];
	return prg_e000[addr - 0xe000];
}

void	vrc2_4_rom::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (vrc2_mode) {
			ram[addr - 0x6000] = data;
		}
		else {
			if (state.ram_enable) ram[addr - 0x6000] = data;
		}
		
	}
	word caddr = recompute_addr(addr);
	if ((caddr >= 0x8000) && (caddr <= 0x8FFF)) {
		state.prgbank0 = data & 0x1F;
		setbanks();
		return;
	}
	if ((caddr >= 0xA000) && (caddr <= 0xAFFF)) {
		state.prgbank1 = data & 0x1F;
		setbanks();
		return;
	}
	if ((caddr >= 0x9000) && (caddr <= 0x9FFF)) {
		if (!vrc2_mode) {
			if (caddr != 0x9002) {
				state.mirror = data & 0x03;
			}
			else {
				state.ram_enable = (data & 0x01) > 0;
				state.sm_fix_8000 = (data & 0x02) > 0;
			}
		}
		else {
			state.mirror = data & 0x01;
		}
		setbanks();
		return;
	}
	// char select.
	if ((caddr >= 0xB000) && (caddr <= 0xEFFF)) {
		byte hi_mask = 0x1F;
		if (vrc2_mode) hi_mask = 0x0F;
		switch (caddr) {
		case 0xB000:
			// CHR0 LO
			state.c[0] = (state.c[0] & 0x1F0) | (data & 0x0F);
			break;
		case 0xB001:
			// CHR0 HI
			state.c[0] = (state.c[0] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xB002:
			// CHR1 LO
			state.c[1] = (state.c[1] & 0x1F0) | (data & 0x0F);
			break;
		case 0xB003:
			// CHR1 HI
			state.c[1] = (state.c[1] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xC000:
			// CHR2 LO
			state.c[2] = (state.c[2] & 0x1F0) | (data & 0x0F);
			break;
		case 0xC001:
			// CHR2 HI
			state.c[2] = (state.c[2] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xC002:
			// CHR3 LO
			state.c[3] = (state.c[3] & 0x1F0) | (data & 0x0F);
			break;
		case 0xC003:
			// CHR3 HI
			state.c[3] = (state.c[3] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xD000:
			// CHR4 LO
			state.c[4] = (state.c[4] & 0x1F0) | (data & 0x0F);
			break;
		case 0xD001:
			// CHR4 HI
			state.c[4] = (state.c[4] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xD002:
			// CHR5 LO
			state.c[5] = (state.c[5] & 0x1F0) | (data & 0x0F);
			break;
		case 0xD003:
			// CHR5 HI
			state.c[5] = (state.c[5] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xE000:
			// CHR6 LO
			state.c[6] = (state.c[6] & 0x1F0) | (data & 0x0F);
			break;
		case 0xE001:
			// CHR6 HI
			state.c[6] = (state.c[6] & 0x0F) | (data & hi_mask) << 4;
			break;
		case 0xE002:
			// CHR7 LO
			state.c[7] = (state.c[7] & 0x1F0) | (data & 0x0F);
			break;
		case 0xE003:
			// CHR7 HI
			state.c[7] = (state.c[7] & 0x0F) | (data & hi_mask) << 4;
			break;
		}
		setbanks();
		return;
	}
	// IRQ
	if ((caddr >= 0xF000) && (caddr <= 0xFFFF)) {
		switch (caddr) {
		case 0xF000:
			// IRQ Latch Lo
			state.irq_latch_reload = (state.irq_latch_reload & 0xF0) | (data & 0x0F);
			break;
		case 0xF001:
			// IRQ Latch Hi
			state.irq_latch_reload = (state.irq_latch_reload & 0x0F) | (data & 0x0F) << 4;
			break;
		case 0xF002:
			// IRQ Control
			irq_enable = false;
			state.irq_start_after_ack = (data & 0x01) > 0;
			state.irq_enabled = (data & 0x02) > 0;
			state.irq_latch = state.irq_latch_reload;
			state.prescaler = 341;
			state.irq_mode = (data & 0x04) >> 2;
			break;
		case 0xF003:
			// IRQ Ack
			irq_enable = false;
			state.irq_enabled = state.irq_start_after_ack;
			break;
		}
		return;
	}
}

void	vrc2_4_rom::reset() {
	memset(&state, 0, sizeof(vrc2_4_state));
}

void	vrc2_4_rom::link_vrom(vrc2_4_vrom *rom) {
	charrom = rom;
}

void	vrc2_4_rom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	reset();
	setbanks();
}

void	vrc2_4_rom::setbanks() {
	// do banking stuff
	if (vrc2_mode) {
		// VRC2
		prg_8000 = &romdata[(state.prgbank0 << 13) % romsize];
		prg_a000 = &romdata[(state.prgbank1 << 13) % romsize];
		prg_c000 = &romdata[romsize - 16384];	// last 16kB @ 0xC000
		prg_e000 = &romdata[romsize - 8192];	// continuation.
	}
	else {
		// VRC4
		if (!state.sm_fix_8000) {
			prg_8000 = &romdata[(state.prgbank0 << 13) % romsize];
			prg_a000 = &romdata[(state.prgbank1 << 13) % romsize];
			prg_c000 = &romdata[romsize - 16384];
			prg_e000 = &romdata[romsize - 8192];
		}
		else {
			prg_8000 = &romdata[romsize - 16384];
			prg_a000 = &romdata[(state.prgbank1 << 13) % romsize];
			prg_c000 = &romdata[(state.prgbank0 << 13) % romsize];
			prg_e000 = &romdata[romsize - 8192];
		}
	}

	// charbanking stuff.
	if (charrom) charrom->setbanks(&state, vrc2a_char_mode);
}

void vrc2_4_rom::set_debug_data() {
	// initialize debugger watchers.
	debugger.add_debug_var("VRC2/4", -1, NULL, t_beginblock);
	debugger.add_debug_var("VRC2 mode", -1, &vrc2_mode, t_bool);
	debugger.add_debug_var("VRC2a char mode", -1, &vrc2a_char_mode, t_bool);
	debugger.add_debug_var("Running as mapper id", -1, &run_as_mapper, t_byte);
	debugger.add_debug_var("NES2 submapper", -1, &submapper, t_byte);
	debugger.add_debug_var("Compat mode", -1, &compability_mode, t_int);
	debugger.add_debug_var("VRC2/4", -1, NULL, t_endblock);

	debugger.add_debug_var("VRC 2/4 state", -1, NULL, t_beginblock);
	debugger.add_debug_var("PRG RAM enable", -1, &state.ram_enable, t_bool);
	debugger.add_debug_var("Fix bank 8000", -1, &state.sm_fix_8000, t_bool);
	debugger.add_debug_var("PRG bank #0", -1, &state.prgbank0, t_byte);
	debugger.add_debug_var("PRG bank #1", -1, &state.prgbank1, t_byte);
	debugger.add_debug_var("Mirror", -1, &state.mirror, t_byte);
	debugger.add_debug_var("Charbanks", 8, &state.c[0], t_wordarray);
	debugger.add_debug_var("IRQ latch", -1, &state.irq_latch, t_byte);
	debugger.add_debug_var("IRQ Reload", -1, &state.irq_latch_reload, t_byte);
	debugger.add_debug_var("IRQ Mode", -1, &state.irq_mode, t_byte);
	debugger.add_debug_var("IRQ Enabled", -1, &state.irq_enabled, t_bool);
	debugger.add_debug_var("IRQ restart after ack", -1, &state.irq_start_after_ack, t_bool);
	debugger.add_debug_var("IRQ Prescaler", -1, &state.prescaler, t_int);
	debugger.add_debug_var("VRC 2/4 state", -1, NULL, t_endblock);
}

/*
		
		CHARROM

*/

vrc2_4_vrom::vrc2_4_vrom() {
	strncpy(get_device_descriptor(), "Denver VRC2/4 Mapper VROM (Multiple)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

void	vrc2_4_vrom::setbanks(vrc2_4_state *state, bool vrc2mode) {
	// VRC4 has 512K! support for charrom.
	if (!romdata) return;

	int shift = 10;
	if (vrc2mode) shift--;

	chr_0000 = &romdata[(state->c[0] << shift) % romsize];
	chr_0400 = &romdata[(state->c[1] << shift) % romsize];
	chr_0800 = &romdata[(state->c[2] << shift) % romsize];
	chr_0c00 = &romdata[(state->c[3] << shift) % romsize];
	chr_1000 = &romdata[(state->c[4] << shift) % romsize];
	chr_1400 = &romdata[(state->c[5] << shift) % romsize];
	chr_1800 = &romdata[(state->c[6] << shift) % romsize];
	chr_1c00 = &romdata[(state->c[7] << shift) % romsize];

	// mirroring.
	switch (state->mirror) {
	case 0:
		ppubus->resetpins_to_default();		// vertical.
		break;
	case 1:
		ppubus->resetpins_to_default();
		ppubus->swappins(10, 11);
		ppubus->groundpin(10);				// horizontal.
		break;
	case 2:
		ppubus->resetpins_to_default();
		ppubus->groundpin(10);				// 1 - screen NT A
		break;
	case 3:
		ppubus->resetpins_to_default();
		ppubus->vccpin(10);					// 1 - screen NT B
		break;
	}
}

byte	vrc2_4_vrom::read(int addr, int addr_from_base, bool onlyread) {
	if (!romdata) return 0x00;
	if ((addr >= 0x0000) && (addr <= 0x03FF)) return chr_0000[addr];
	if ((addr >= 0x0400) && (addr <= 0x07FF)) return chr_0400[addr - 0x0400];
	if ((addr >= 0x0800) && (addr <= 0x0BFF)) return chr_0800[addr - 0x0800];
	if ((addr >= 0x0c00) && (addr <= 0x0FFF)) return chr_0c00[addr - 0x0C00];
	if ((addr >= 0x1000) && (addr <= 0x13FF)) return chr_1000[addr - 0x1000];
	if ((addr >= 0x1400) && (addr <= 0x17FF)) return chr_1400[addr - 0x1400];
	if ((addr >= 0x1800) && (addr <= 0x1BFF)) return chr_1800[addr - 0x1800];
	return chr_1c00[addr - 0x1C00];
}

void	vrc2_4_vrom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
}
