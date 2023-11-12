/*

	implementation of the vrc6 rom mapper.

*/

#pragma warning(disable : 4996)

#include "mapper_024026.h"
#include <cstdlib>
#include <iostream>

// ROM

vrc6rom::vrc6rom() {
	strncpy(get_device_descriptor(), "Denver VRC6 ROM (mapper 024/026)", MAX_DESCRIPTOR_LENGTH);
	ram = (char*)malloc(8192);
	memset(&state, 0x00, sizeof(vrc6_state));
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
}

vrc6rom::~vrc6rom() {
	free(ram);
}

void	vrc6rom::reset() {
	memset(&state, 0x00, sizeof(vrc6_state));
}

byte	vrc6rom::read(int addr, int addr_from_base, bool onlyread) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (state.prg_ram_enable) return ram[addr - 0x6000];
		return 0x00;
	}
	if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg_8000[addr - 0x8000];
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) return prg_c000[addr - 0xC000];
	return prg_e000[addr - 0xE000];
}

void	vrc6rom::write(int addr, int addr_from_base, byte data) {
	word taddr = addr;
	if (mapper_026) {
		word a0 = (addr & 1) << 1;
		word a1 = (addr & 2) >> 1;
		taddr = addr & ~3;
		taddr = taddr | a0 | a1;
	}

	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (state.prg_ram_enable) ram[addr - 0x6000] = data;
		return;
	}

	if ((taddr >= 0x8000) && (taddr <= 0x8003)) {
		// set prg16 bank.
		state.prg1_bank = data & 0x0F;
		setbanks();
		return;
	}

	if ((taddr >= 0xC000) && (taddr <= 0xC003)) {
		// set prg8 bank.
		state.prg2_bank = data & 0x1F;
		setbanks();
		return;
	}

	if (taddr == 0xB003) {
		state.mode = data & 0x03;	// is ignored in commercial games.
		state.mirror = (data & 0x0C) >> 2;	// mirror mode.
		state.prg_ram_enable = (data & 0x80) > 0;
		state.chr10 = (data & 0x20) > 0;
		setbanks();
		return;
	}

	if ((taddr >= 0xD000) && (taddr <= 0xD003)) {
		// R0..R3 set.
		byte selector = taddr & 0x03;
		state.r[selector] = data;
		setbanks();
		return;
	}

	if ((taddr >= 0xE000) && (taddr <= 0xE003)) {
		// R4..R7 set.
		byte selector = taddr & 0x03;
		state.r[4 + selector] = data;
		setbanks();
		return;
	}

	if ((taddr >= 0xF000) && (taddr <= 0xF003)) {
		byte selector = taddr & 0x03;
		switch (selector) {
		case 0:
			state.irq_latch_reload = data;
			return;
			break;
		case 1:
			irq_enable = false;
			state.irq_start_after_ack = (data & 0x01) > 0;
			state.irq_enabled = (data & 0x02) > 0;
			state.irq_latch = state.irq_latch_reload;
			state.prescaler = 341;
			state.irq_mode = (data & 0x04) >> 2;
			return;
			break;
		case 2:
			irq_enable = false;
			state.irq_enabled = state.irq_start_after_ack;
			return;
			break;
		}
	}
}

int		vrc6rom::rundevice(int ticks) {
	if (audiochip) audiochip->rundevice(ticks);
	if (!state.irq_enabled) return ticks;

	if (state.irq_mode == 0) {
		for (int i = 0; i < ticks; i++) {
			state.prescaler -= 3;
			if (state.prescaler <= 0) {
				state.prescaler += 341;
				if (state.irq_latch == 0xFF) {
					irq_enable = true;	// raise IRQ.		
					state.irq_latch = state.irq_latch_reload;
				} else {
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

void	vrc6rom::link_vrom(vrc6vrom *rom) {
	charrom = rom;
}

void	vrc6rom::setbanks() {
	// set program banks.
	prg_e000 = &romdata[romsize - 8192];	// last 8kB on 0xE000
	prg_8000 = &romdata[(state.prg1_bank << 14) % romsize];
	prg_c000 = &romdata[(state.prg2_bank << 13) % romsize];
	if (charrom) charrom->setbanks(&state);
}

void	vrc6rom::set_rom_data(byte *data, std::size_t size) {
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;

	// reset state.
	memset(&state, 0x00, sizeof(vrc6_state));
	setbanks();
}

// VROM

vrc6vrom::vrc6vrom() {
	strncpy(get_device_descriptor(), "Denver VRC6 VROM (mapper 024/026)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

void	vrc6vrom::setbanks(vrc6_state *state) {
	// we only support mode #0 (because mode 1 up to 7 are not used in games)
	if (!romdata) return;
	switch (state->mode) {
	case 0:
		chr_0000 = &romdata[(state->r[0] << 10) % romsize];
		chr_0400 = &romdata[(state->r[1] << 10) % romsize];
		chr_0800 = &romdata[(state->r[2] << 10) % romsize];
		chr_0c00 = &romdata[(state->r[3] << 10) % romsize];
		chr_1000 = &romdata[(state->r[4] << 10) % romsize];
		chr_1400 = &romdata[(state->r[5] << 10) % romsize];
		chr_1800 = &romdata[(state->r[6] << 10) % romsize];
		chr_1c00 = &romdata[(state->r[7] << 10) % romsize];
		break;
	case 1:
		chr_0000 = &romdata[(state->r[0] << 10) % romsize];
		chr_0400 = &romdata[(state->r[0] << 10) % romsize];
		chr_0800 = &romdata[(state->r[1] << 10) % romsize];
		chr_0c00 = &romdata[(state->r[1] << 10) % romsize];
		chr_1000 = &romdata[(state->r[2] << 10) % romsize];
		chr_1400 = &romdata[(state->r[2] << 10) % romsize];
		chr_1800 = &romdata[(state->r[3] << 10) % romsize];
		chr_1c00 = &romdata[(state->r[3] << 10) % romsize];
		break;
	default:
		chr_0000 = &romdata[(state->r[0] << 10) % romsize];
		chr_0400 = &romdata[(state->r[1] << 10) % romsize];
		chr_0800 = &romdata[(state->r[2] << 10) % romsize];
		chr_0c00 = &romdata[(state->r[3] << 10) % romsize];
		chr_1000 = &romdata[(state->r[4] << 10) % romsize];
		chr_1400 = &romdata[(state->r[4] << 10) % romsize];
		chr_1800 = &romdata[(state->r[5] << 10) % romsize];
		chr_1c00 = &romdata[(state->r[5] << 10) % romsize];
		break;
	}

	// mirror states.
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

void	vrc6vrom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;	
}

byte	vrc6vrom::read(int addr, int addr_from_base, bool onlyread) {
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