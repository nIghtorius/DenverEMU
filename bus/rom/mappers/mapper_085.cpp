/*

	VRC7 mapper implementation.

*/

#include "mapper_085.h"
#include <cstdlib>
#include <iostream>

#pragma warning(disable : 4996)

// ROM
vrc7rom::vrc7rom() {
	strncpy(get_device_descriptor(), "Denver VRC7 ROM (mapper 085)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	ram = (char*)malloc(8192);
}

vrc7rom::~vrc7rom() {
	free(ram);
}

batterybackedram* vrc7rom::get_battery_backed_ram() {
	return new batterybackedram((byte*)ram, 8192);
}

void vrc7rom::set_battery_backed_ram(byte* data, std::size_t size) {
	if (size > 8192) return;
	memcpy(ram, data, size);
}

void vrc7rom::reset() {
	memset(&state, 0x00, sizeof(vrc7_state));
}

void vrc7rom::link_vrom(vrc7vrom* rom) {
	charrom = rom;
}

void vrc7rom::set_rom_data(byte* data, std::size_t size) {
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;

	// reset state.
	memset(&state, 0x00, sizeof(vrc7_state));
	setbanks();
}


int vrc7rom::rundevice(int ticks) {
	if (audiochip) audiochip->rundevice(ticks);
	if (!state.irq_enabled) return ticks;

	if (state.irq_mode == 0) {
		for (int i = 0; i < ticks; i++) {
			state.prescaler -= 3;
			if (state.prescaler < -8) {
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

void vrc7rom::setbanks() {
	// set program banks.
	prg_e000 = &romdata[romsize - 8192];	// last 8kB on 0xE000
	prg_8000 = &romdata[(state.prgrom[0] << 13) % romsize];
	prg_a000 = &romdata[(state.prgrom[1] << 13) % romsize];
	prg_c000 = &romdata[(state.prgrom[2] << 13) % romsize];
	if (charrom) charrom->setbanks(&state);
}

byte vrc7rom::read(int addr, int addr_from_base, bool onlyread) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		return ram[addr - 0x6000];
	}
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) return prg_8000[addr - 0x8000];
	if ((addr >= 0xA000) && (addr <= 0xBFFF)) return prg_a000[addr - 0xA000];
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) return prg_c000[addr - 0xC000];
	return prg_e000[addr - 0xE000];
}

void vrc7rom::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (state.wram_enabled) ram[addr - 0x6000] = data;
		return;
	}

	if (addr == 0x8000) {
		// PRG Select 0
		state.prgrom[0] = data & 0x3F;
		setbanks();
		return;
	}
	if ((addr == 0x8010) || (addr == 0x8008)) {
		// PRG Select 1
		state.prgrom[1] = data & 0x3F;
		setbanks();
		return;
	}
	if (addr == 0x9000) {
		// PRG select 2
		state.prgrom[2] = data & 0x3F;
		setbanks();
	}
	// char banks.
	switch (addr) {
	case 0xA000:
		state.chrrom[0] = data;
		setbanks();
		return;
		break;
	case 0xA008:
	case 0xA010:
		state.chrrom[1] = data;
		setbanks();
		return;
		break;
	case 0xB000:
		state.chrrom[2] = data;
		setbanks();
		return;
		break;
	case 0xB008:
	case 0xB010:
		state.chrrom[3] = data;
		setbanks();
		return;
		break;
	case 0xC000:
		state.chrrom[4] = data;
		setbanks();
		return;
		break;
	case 0xC008:
	case 0xC010:
		state.chrrom[5] = data;
		setbanks();
		return;
		break;
	case 0xD000:
		state.chrrom[6] = data;
		setbanks();
		return;
		break;
	case 0xD008:
	case 0xD010:
		state.chrrom[7] = data;
		setbanks();
		return;
		break;
	}
	// MIRROR
	if (addr == 0xE000) {
		state.wram_enabled = (data & 0x80) > 0;
		state.mirror = data & 0x03;
		setbanks();
		return;
	}
	// IRQ
	if ((addr == 0xE008) || (addr == 0xE010)) {
		state.irq_latch_reload = data;
		return;
	}
	if (addr == 0xF000) {
		irq_enable = false;
		state.irq_start_after_ack = (data & 0x01) > 0;
		state.irq_enabled = (data & 0x02) > 0;
		state.irq_latch = state.irq_latch_reload;
		state.prescaler = 341;
		state.irq_mode = (data & 0x04) >> 2;
		return;
	}
	if ((addr == 0xF008) || (addr == 0xF010)) {
		irq_enable = false;
		state.irq_enabled = state.irq_start_after_ack;
		return;
	}
}

// VROM

vrc7vrom::vrc7vrom() {
	strncpy(get_device_descriptor(), "Denver VRC7 VROM (mapper 085)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
	vram = (char*)malloc(262144);
}

vrc7vrom::~vrc7vrom() {
	if (vram) free(vram);
}

void vrc7vrom::switch_vram() {
	romdata = (byte*)vram;
	romsize = 262144;
}

void	vrc7vrom::setbanks(vrc7_state *state) {
	if (!romdata) return;

	chr_0000 = &romdata[(state->chrrom[0] << 10) % romsize];
	chr_0400 = &romdata[(state->chrrom[1] << 10) % romsize];
	chr_0800 = &romdata[(state->chrrom[2] << 10) % romsize];
	chr_0c00 = &romdata[(state->chrrom[3] << 10) % romsize];
	chr_1000 = &romdata[(state->chrrom[4] << 10) % romsize];
	chr_1400 = &romdata[(state->chrrom[5] << 10) % romsize];
	chr_1800 = &romdata[(state->chrrom[6] << 10) % romsize];
	chr_1c00 = &romdata[(state->chrrom[7] << 10) % romsize];

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

void	vrc7vrom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

byte	vrc7vrom::read(int addr, int addr_from_base, bool onlyread) {
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

void	vrc7vrom::write(int addr, int addr_from_base, byte data) {
	if (romdata != (byte*)vram) return; // ROM mode.
	if ((addr >= 0x0000) && (addr <= 0x03FF)) chr_0000[addr] = data;
	if ((addr >= 0x0400) && (addr <= 0x07FF)) chr_0400[addr - 0x0400] = data;
	if ((addr >= 0x0800) && (addr <= 0x0BFF)) chr_0800[addr - 0x0800] = data;
	if ((addr >= 0x0c00) && (addr <= 0x0FFF)) chr_0c00[addr - 0x0C00] = data;
	if ((addr >= 0x1000) && (addr <= 0x13FF)) chr_1000[addr - 0x1000] = data;
	if ((addr >= 0x1400) && (addr <= 0x17FF)) chr_1400[addr - 0x1400] = data;
	if ((addr >= 0x1800) && (addr <= 0x1BFF)) chr_1800[addr - 0x1800] = data;
	if (addr >= 0x1C00) chr_1c00[addr - 0x1C00] = data;
}