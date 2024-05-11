/*
	
	implementation of the mmc2/4 mapper.

*/

#pragma warning(disable : 4996)

#include "mapper_009.h"
#include <iostream>

// ROM

mmc2_rom::mmc2_rom() {
	strncpy(get_device_descriptor(), "Denver MMC2/4 mapper ROM", MAX_DESCRIPTOR_LENGTH);
	// allocate prgram (0x6000-0x7FFF)
	prgram6000 = (byte *)malloc(0x2000);
	memset(&state, 0, sizeof(mmc2_state));
	devicestart = 0x6000;
	devicemask = 0xFFFF;
	deviceend = 0xFFFF;
}

mmc2_rom::~mmc2_rom() {
	free(prgram6000);
}

batterybackedram* mmc2_rom::get_battery_backed_ram() {
	return new batterybackedram((byte*)prgram6000, 8192);
}

void mmc2_rom::set_battery_backed_ram(byte* data, const std::size_t size) {
	if (size > 8192) return;
	memcpy(prgram6000, data, size);
}

byte	mmc2_rom::read(const int addr, const int addr_from_base, const bool onlyread) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) return prgram6000[addr - 0x6000];
	if (!mmc4mode) {
		if ((addr >= 0x8000) && (addr <= 0x9FFF)) return prg8000[addr - 0x8000];
		return romdata[romsize - 24576 + (addr - 0xA000)];
	}
	else {
		if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg8000[addr - 0x8000];
		return romdata[romsize - 16384 + (addr - 0xC000)];
	}
}

void	mmc2_rom::write(const int addr, const int addr_from_base, const byte data) {
	if ((addr >= 0xA000) && (addr <= 0xAFFF)) {
		state.prg_bnk = data & 0x0F;
		update_banks();
		return;
	}
	if ((addr >= 0xB000) && (addr <= 0xBFFF)) {
		state.chr_fd_1_bnk = data & 0x1F;
		update_banks();
		return;
	}
	if ((addr >= 0xC000) && (addr <= 0xCFFF)) {
		state.chr_fe_1_bnk = data & 0x1F;
		update_banks();
		return;
	}
	if ((addr >= 0xD000) && (addr <= 0xDFFF)) {
		state.chr_fd_2_bnk = data & 0x1F;
		update_banks();
		return;
	}
	if ((addr >= 0xE000) && (addr <= 0xEFFF)) {
		state.chr_fe_2_bnk = data & 0x1F;
		update_banks();
		return;
	}
	if (addr >= 0xF000) {
		state.horimirror = (data & 0x01) > 0;
	}
}

void	mmc2_rom::set_rom_data(byte *data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
	update_banks();
}

void	mmc2_rom::link_vrom(mmc2_vrom *lvrom) {
	vrom = lvrom;
	update_banks();
}

void	mmc2_rom::update_banks() {
	if (!mmc4mode) {
		prg8000 = &romdata[(state.prg_bnk << 13) % romsize];
	}
	else {
		prg8000 = &romdata[(state.prg_bnk << 14) % romsize];
	}
	if (vrom) vrom->update_banks(&state);
}

// VROM

mmc2_vrom::mmc2_vrom() {
	strncpy(get_device_descriptor(), "Denver MMC2 Mapper VROM", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

void	mmc2_vrom::update_banks(mmc2_state *state) {
	// set chrroms.
	chrfd0000 = &romdata[(state->chr_fd_1_bnk << 12) % romsize];
	chrfe0000 = &romdata[(state->chr_fe_1_bnk << 12) % romsize];
	chrfd1000 = &romdata[(state->chr_fd_2_bnk << 12) % romsize];
	chrfe1000 = &romdata[(state->chr_fe_2_bnk << 12) % romsize];
	// check mirroring.
	if (state->horimirror) {
		ppubus->resetpins_to_default();
		ppubus->swappins(10, 11);
		ppubus->groundpin(10);
	}
	else {
		ppubus->resetpins_to_default();
	}
}

void	mmc2_vrom::set_rom_data(byte *data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
}

byte	mmc2_vrom::read(const int addr, const int addr_from_base, const bool onlyread) {
	byte data = 0x00;
	if (addr < 0x1000) {
		data = l0fe ? chrfe0000[addr] : chrfd0000[addr];
	}
	else {
		data = l1fe ? chrfe1000[addr - 0x1000] : chrfd1000[addr - 0x1000];
	}
	if (!onlyread) {
		// bank switching.
		if (addr == 0x0FD8) {
			l0fe = false;
		}
		if (addr == 0x0FE8) {
			l0fe = true;
		}
		if ((addr >= 0x1FD8) && (addr <= 0x1FDF)) {
			l1fe = false;
		}
		if ((addr >= 0x1FE8) && (addr <= 0x1FEF)) {
			l1fe = true;
		}
	}
	return data;
}
