#include "mapper_001.h"
#include <iostream>

#pragma warning(disable : 4996)

// implementation of MMC1 (mapper_001.h)

// ROM side.
mmc1_rom::mmc1_rom() {
	strncpy(get_device_descriptor(), "Denver MMC1 (mapper 001)", MAX_DESCRIPTOR_LENGTH);
	mmc1ram = (char *)malloc(8192);
}

mmc1_rom::~mmc1_rom() {
	free(mmc1ram);
}

byte mmc1_rom::read(int addr, int addr_from_base, bool onlyread)
{
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) return mmc1ram[addr - 0x6000];
	if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg8000[addr - 0x8000];
	return prgC000[addr - 0xC000];
}

void mmc1_rom::update_control() {
	// parse control register.
	state.mirroring = state.control & 0x03;
	state.prg_bank_mode = (state.control & 0x0C) >> 2;
	state.chr_bank_mode = (state.control & 0x10) >> 4;
}

void mmc1_rom::update_banks() {
	int		bank = 0;
	switch (state.prg_bank_mode) {
	case MMC1_PRG_32K_MODE:
	case MMC1_PRG_32K_MODE2:
		bank = state.prgbank >> 1;
		prg8000 = &romdata[(bank << 15)%(romsize)];
		prgC000 = &romdata[(bank << 15)%(romsize) | 0x4000];
		break;
	case MMC1_PRG_FIRSTBANK_FIX_8000:
		bank = state.prgbank;
		prg8000 = &romdata[0];
		prgC000 = &romdata[(bank << 14)%(romsize)];
		break;
	case MMC1_PRG_LASTBANK_FIX_C000:
		bank = state.prgbank;
		prg8000 = &romdata[(bank << 14)%(romsize)];
		prgC000 = &romdata[romsize - 0x4000];
		break;
	}
	// update linked vrom too.
	if (charrom) charrom->update_banks(state);
}

void mmc1_rom::write(int addr, int addr_from_base, byte data)
{
	if (addr < 0x8000) {
		mmc1ram[addr - 0x6000] = data;
		return;
	}
	if (data & 0x80) {
		state.control |= 0x0C;
		update_control();
		update_banks();
		state.shift_cnt = 0x04;
		state.shift_reg = 0x00;
		return;
	}
	
	// fill shift register. (5 bits)
	state.shift_reg >>= 1;
	state.shift_reg |= (data & 1) << 4;
	state.shift_cnt--;
	if (state.shift_cnt != 0xFF) return;
	
	// shift register is filled with 5 bits, process.
	state.shift_cnt = 0x04;			// reset counter for the shift register again.
	int	function = (addr & 0x6000) >> 13;
	
	switch (function) {
	case MMC1_CONTROL:
		state.control = state.shift_reg;
		update_control();
		break;
	case MMC1_CHRBANK0:
		state.chrbank0 = state.shift_reg;
		update_banks();
		break;
	case MMC1_CHRBANK1:
		state.chrbank1 = state.shift_reg;
		update_banks();
		break;
	case MMC1_PRGBANK:
		state.prgbank = state.shift_reg;
		update_banks();
		break;
	}	
}

void mmc1_rom::set_rom_data(byte *data, std::size_t size)
{
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;

	// reset state is control = 0x0c
	state.control = 0x0c;
	update_control();
	update_banks();
}

void mmc1_rom::link_vrom(mmc1_vrom *vrom) {
	charrom = vrom;
}

// VROM side.
mmc1_vrom::mmc1_vrom() {
	strncpy(this->get_device_descriptor(), "Denver MMC1 (mapper 001) - VROM", MAX_DESCRIPTOR_LENGTH);
	ram = (byte *)malloc(8192);	// reserve 8192 of RAM for MMC1 (may not be used, can be used)
}

mmc1_vrom::~mmc1_vrom() {
	free(ram);
}

byte mmc1_vrom::read(int addr, int addr_from_base, bool onlyread)
{
	if (ram_mode) return ram[addr_from_base];
	if (addr < 0x1000) return chr0000[addr];
	return chr1000[addr - 0x1000];
}

void mmc1_vrom::write(int addr, int addr_from_base, byte data)
{
	// write when in RAM_MODE.
	if (ram_mode) ram[addr_from_base] = data;
}

void mmc1_vrom::set_rom_data(byte *data, std::size_t size)
{
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
	romdata = data;
	romsize = (int)size;
	chr0000 = romdata;
	chr1000 = romdata;
}

void mmc1_vrom::update_banks(mmc1_state &state) {
	if (ppubus) {
		switch (state.mirroring) {
		case MMC1_MIRROR_ONESCRN_LOWER:
			ppubus->resetpins_to_default();
			ppubus->groundpin(10);
			ppubus->groundpin(11);
			break;
		case MMC1_MIRROR_ONESCRN_UPPER:
			ppubus->resetpins_to_default();
			ppubus->groundpin(10);
			ppubus->groundpin(11);
			break;
		case MMC1_MIRROR_VERTICAL:
			ppubus->resetpins_to_default();
			break;
		case MMC1_MIRROR_HORIZONTAL:
			ppubus->resetpins_to_default();
			ppubus->swappins(10, 11);
			ppubus->groundpin(10);
			break;
		}
	}
	if (ram_mode) return;	// no need to switch (we are in RAM mode)
	int bank = 0;
	switch (state.chr_bank_mode) {
	case MMC1_CHR_8K_MODE:
		bank = state.chrbank0 >> 1;
		chr0000 = &romdata[(bank << 13)%(romsize-4096)]; // 8 kB blocks.
		chr1000 = &romdata[(bank << 13)%(romsize-4096) | 0x1000];
		break;
	case MMC1_CHR_4K_MODE:
		bank = state.chrbank0;
		chr0000 = &romdata[(bank << 12)%(romsize)]; // 4kB blocks.
		bank = state.chrbank1;
		chr1000 = &romdata[(bank << 12)%(romsize)]; // 4kB blocks.
		break;
	}
}

void mmc1_vrom::is_ram(bool enable) {
	ram_mode = enable;
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}
