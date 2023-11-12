/*

	Implementation of the FME-7 mapper. (Sunsoft)

*/

#include "mapper_069.h"
#include <iostream>

#pragma warning(disable : 4996)

fme7rom::fme7rom() {
	strncpy(get_device_descriptor(), "Denver FME-7 ROM (Mapper 069)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	// prgram (max 256kB)
	ram = (byte*)malloc(262144);
	reset();
}

fme7rom::~fme7rom() {
	free(ram);
}

void	fme7rom::reset() {
	memset(&state, 0, sizeof(fme7_state));
}

int		fme7rom::rundevice(int ticks) {
	// expansion audio?
	if (audiochip) audiochip->rundevice(ticks);
	// do irq counters.
	if (state.irq_counter_enable) {
		for (int i = 0; i < ticks; i++) {
			state.irq_counter--;
			if (state.irq_counter == 0xFFFF) {
				if (state.irq_enable) irq_enable = true;
			}
		}
	}
	return ticks;
}

void	fme7rom::link_vrom(fme7vrom *rom) {
	charrom = rom;
}

byte	fme7rom::read(int addr, int addr_from_base, bool onlyread) {
	// prg ram or rom?
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		if (state.ramrom_select) {
			return state.prg_ram_enable ? prg_6000[addr - 0x6000] : 0;
		}
		else {
			return prg_6000[addr - 0x6000];
		}
	}
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) return prg_8000[addr - 0x8000];
	if ((addr >= 0xA000) && (addr <= 0xBFFF)) return prg_a000[addr - 0xA000];
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) return prg_c000[addr - 0xC000];
	return prg_e000[addr - 0xE000];
}

void	fme7rom::write(int addr, int addr_from_base, byte data) {
	// prg ram?
	if (state.prg_ram_enable && state.ramrom_select) {
		if ((addr >= 0x6000) && (addr <= 0x7FFF)) prg_6000[addr - 0x6000] = data;
		return;
	}
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) {
		// command register.
		state.cmdreg = data & 0x0F;
		return;
	}
	if ((addr >= 0xA000) && (addr <= 0xBFFF)) {
		// parameter register.
		switch (state.cmdreg) {
		case FME7_CMD_CHRBANK0:
		case FME7_CMD_CHRBANK1:
		case FME7_CMD_CHRBANK2:
		case FME7_CMD_CHRBANK3:
		case FME7_CMD_CHRBANK4:
		case FME7_CMD_CHRBANK5:
		case FME7_CMD_CHRBANK6:
		case FME7_CMD_CHRBANK7:
			state.c[state.cmdreg] = data;
			setbanks();
			break;
		case FME7_CMD_PRGBANK0:
			state.prg_ram_enable = (data & 0x80) > 0;
			state.ramrom_select = (data & 0x40) > 0;
			state.prg0_select = (data & 0x3F);
			setbanks();
			break;
		case FME7_CMD_PRGBANK1:
		case FME7_CMD_PRGBANK2:
		case FME7_CMD_PRGBANK3:
			state.p[state.cmdreg - FME7_CMD_PRGBANK1] = data & 0x3F;
			setbanks();
			break;
		case FME7_CMD_MIRRORING:
			state.mirroring = data & 0x03;
			break;
		case FME7_CMD_IRQCTRL:
			state.irq_enable = (data & 0x01) > 0;
			state.irq_counter_enable = (data & 0x80) > 0;
			irq_enable = false;		// ack irq.
			break;
		case FME7_CMD_IRQCLO:
			state.irq_counter = (state.irq_counter & 0xFF00) | data;
			break;
		case FME7_CMD_IRQCHI:
			state.irq_counter = (state.irq_counter & 0x00FF) | (data << 8);
			break;
		}
		return;
	}
}

void	fme7rom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	setbanks();
}

void	fme7rom::setbanks() {
	// prg 0 bank is a doozy.
	// it supports also PRGRAM bank switching (up to 256kB)
	if (state.ramrom_select) {
		// RAM mode.
		prg_6000 = &ram[(state.prg0_select << 13) % prgsize];
	}
	else {
		// ROM mode.
		prg_6000 = &romdata[(state.prg0_select << 13) % romsize];
	}
	prg_8000 = &romdata[(state.p[0] << 13) % romsize];
	prg_a000 = &romdata[(state.p[1] << 13) % romsize];
	prg_c000 = &romdata[(state.p[2] << 13) % romsize];
	prg_e000 = &romdata[romsize - 8192];
	
	if (charrom) charrom->setbanks(&state);
}

fme7vrom::fme7vrom() {
	strncpy(get_device_descriptor(), "Denver FME-7 VROM (Mapper 069)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

fme7vrom::~fme7vrom() {

}

void	fme7vrom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
}

void	fme7vrom::setbanks(fme7_state *state) {
	if (!romdata) return;
	chr_0000 = &romdata[(state->c[0] << 10) % romsize];
	chr_0400 = &romdata[(state->c[1] << 10) % romsize];
	chr_0800 = &romdata[(state->c[2] << 10) % romsize];
	chr_0c00 = &romdata[(state->c[3] << 10) % romsize];
	chr_1000 = &romdata[(state->c[4] << 10) % romsize];
	chr_1400 = &romdata[(state->c[5] << 10) % romsize];
	chr_1800 = &romdata[(state->c[6] << 10) % romsize];
	chr_1c00 = &romdata[(state->c[7] << 10) % romsize];
	// mirroring.
	switch (state->mirroring) {
	case 0:
		ppubus->resetpins_to_default();		// vertical.
		ppubus->groundpin(11);
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

byte	fme7vrom::read(int addr, int addr_from_base, bool onlyread) {
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