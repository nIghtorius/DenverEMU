#include "mapper_004.h"
#include "../../video/ppu.h"
#include <iostream>

#pragma warning(disable : 4996)

// implementation of MMC3 (mapper_004.h)

// rom side.
mmc3_rom::mmc3_rom() {
	strncpy(get_device_descriptor(), "Denver MMC3 (mapper 004)", MAX_DESCRIPTOR_LENGTH);
	prgram6000 = (byte *)malloc(8192);
}

mmc3_rom::~mmc3_rom() {
	free(prgram6000);
}

byte	mmc3_rom::read(int addr, int addr_from_base, bool onlyread)
{
	if ((addr >= 0x6000) && (addr <= 0x7fff)) {
		return state.prg_ram_enable ? prgram6000[addr - 0x6000] : 0;
	}
	if ((addr >= 0x8000) && (addr <= 0x9fff)) return prg8000[addr - 0x8000];
	if ((addr >= 0xa000) && (addr <= 0xbfff)) return prga000[addr - 0xa000];
	if ((addr >= 0xc000) && (addr <= 0xdfff)) return prgc000[addr - 0xc000];
	return prge000[addr - 0xe000];
}

void	mmc3_rom::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		//if (!state.prg_ram_enable) return;
		//if (state.prg_ram_readonly) return;
		prgram6000[addr - 0x6000] = data;
		return;
	}
	bool odd = addr & 1;
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) {
		if (!odd) {
			state.bank_update_reg = data & MMC3_BANKUPDATE_REG;
			state.prg_bank_mode = (data & MMC3_BANK_MODE) > 0;
			state.chr_a12_inv = (data & MMC3_CHR_A12_INVERSION) > 0;
			return;
		}
		else {
			write_banks(data);
			update_banks();
			return;
		}
	}
	if ((addr >= 0xA000) && (addr <= 0xBFFF)) {
		if (!odd) {
			// Mirroring control
			state.do_horizontal_mirroring = (data & MMC3_HORIZONTAL_MIRRORING) > 0;
			if (vrom) vrom->update_banks(&state);
			return;
		}
		else {
			// program ram control
			state.prg_ram_enable = (data & MMC3_PROGRAM_RAM_ENABLE) > 0;
			state.prg_ram_readonly = (data & MMC3_PROGRAM_RAM_RO) > 0;
			return;
		}
	}
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) {
		if (!odd) {
			// IRQ latch.
			state.irq_latch = data;
			return;
		} else {
			// IRQ reload.
			state.irq_reload = true;
		}
	}
	if ((addr >= 0xE000) && (addr <= 0xFFFF)) {
		if (!odd) {
			// IRQ disable.
			state.irq_enable = false;
			irq_enable = false;
		}
		else {
			// IRQ enable.
			state.irq_enable = true;
		}
	}
}

void	mmc3_rom::write_banks(byte data) {
	switch (state.bank_update_reg) {
	case 0x00:
		// R0: Select 2 KB CHR bank at PPU $0000-$07FF (or $1000-$17FF)
		state.r0 = data;
		break;
	case 0x01:
		// R1: Select 2 KB CHR bank at PPU $0800-$0FFF (or $1800-$1FFF)
		state.r1 = data;
		break;
	case 0x02:
		// R2: Select 1 KB CHR bank at PPU $1000-$13FF (or $0000-$03FF)
		state.r2 = data;
		break;
	case 0x03:
		// R3: Select 1 KB CHR bank at PPU $1400-$17FF (or $0400-$07FF)
		state.r3 = data;
		break;
	case 0x04:
		// R4: Select 1 KB CHR bank at PPU $1800 - $1BFF(or $0800 - $0BFF)
		state.r4 = data;
		break;
	case 0x05:
		// R5: Select 1 KB CHR bank at PPU $1C00 - $1FFF(or $0C00 - $0FFF)
		state.r5 = data;
		break;
	case 0x06:
		// R6: Select 8 KB PRG ROM bank at $8000 - $9FFF(or $C000 - $DFFF)
		state.r6 = data & 0x3F;
		break;
	case 0x07:
		// R7: Select 8 KB PRG ROM bank at $A000-$BFFF
		state.r7 = data & 0x3F;
		break;
	}
}

void	mmc3_rom::update_banks() {
	// register 6
	if (!state.prg_bank_mode) {
		// R6 => 0x8000-0x9FFF
		prg8000 = &romdata[(state.r6 << 13)%romsize];
		prgc000 = &romdata[romsize - 16384];
	}
	else {
		// R6 => 0xC000-0xDFFF
		prgc000 = &romdata[(state.r6 << 13)%romsize];
		prg8000 = &romdata[romsize - 16384];
	}
	// register 7
	prga000 = &romdata[(state.r7 << 13)%romsize];
	prge000 = &romdata[romsize - 8192];

	// trigger the event to mmc3_vrom
	if (vrom) vrom->update_banks(&state);
}

void	mmc3_rom::set_rom_data(byte *data, std::size_t size) {
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;

	// reset state.
	memset(&state, 0, sizeof(mmc3_state));
	state.prg_ram_enable = true;
	state.prg_ram_readonly = false;
	update_banks();
}

void	mmc3_rom::link_vrom(mmc3_vrom * m3vrom) {
	vrom = m3vrom;
	if (vrom) {
		vrom->update_banks(&state);
	}
}

int		mmc3_rom::rundevice(int ticks) {
	if (!vrom) return ticks;
	word ppuaddr = vrom->fetch_ppu_addr();
	bool a12_risen = ((ppuaddr & 0x1000) > 0) && ((lastppuaddr & 0x1000) == 0);	
	if (a12_risen) {
		if ((state.irq_counter == 0) || (state.irq_reload)) {
			state.irq_counter = state.irq_latch;
			state.irq_reload = false;
		}
		else {
			state.irq_counter--;
		}
		if ((state.irq_counter == 0) && (state.irq_enable)) {
			irq_enable = true;
		}
	}
	lastppuaddr = ppuaddr;
	return ticks;
}

mmc3_vrom::mmc3_vrom() {
	strncpy(get_device_descriptor(), "Denver MMC3 VROM (mapper 004)", MAX_DESCRIPTOR_LENGTH);
}

word mmc3_vrom::fetch_ppu_addr() {
	return ppuaddr;
}

void mmc3_vrom::update_banks(mmc3_state *state) {
	if (!romdata) return;
	if (!state->chr_a12_inv) {
		chr0000 = &romdata[((state->r0 & 0xFE) << 10) % romsize];
		chr0400 = &romdata[(((state->r0 & 0xFE) << 10) + 1024) % romsize];
		chr0800 = &romdata[((state->r1 & 0xFE) << 10) % romsize];
		chr0c00 = &romdata[(((state->r1 & 0xFE) << 10) + 1024) % romsize];
		chr1000 = &romdata[(state->r2 << 10)%romsize];
		chr1400 = &romdata[(state->r3 << 10)%romsize];
		chr1800 = &romdata[(state->r4 << 10)%romsize];
		chr1c00 = &romdata[(state->r5 << 10)%romsize];
	}
	else {
		chr0000 = &romdata[(state->r2 << 10)%romsize];
		chr0400 = &romdata[(state->r3 << 10)%romsize];
		chr0800 = &romdata[(state->r4 << 10)%romsize];
		chr0c00 = &romdata[(state->r5 << 10)%romsize];
		chr1000 = &romdata[((state->r0 & 0xFE) << 10) % romsize];
		chr1400 = &romdata[(((state->r0 & 0xFE) << 10) + 1024) % romsize];
		chr1800 = &romdata[((state->r1 & 0xFE) << 10) % romsize];
		chr1c00 = &romdata[(((state->r1 & 0xFE) << 10) + 1024) % romsize];
	}

	// mirroring.
	if (state->do_horizontal_mirroring) {
		ppubus->resetpins_to_default();
		ppubus->swappins(10, 11);
		ppubus->groundpin(10);
	}
	else {
		ppubus->resetpins_to_default();
	}
}

byte mmc3_vrom::read(int addr, int addr_from_base, bool onlyread) {
	if (!onlyread) ppuaddr = addr;
	if (!chr0000) return 0;	// not initialized yet.
	if ((addr >= 0x0000) && (addr <= 0x03ff)) return chr0000[addr];
	if ((addr >= 0x0400) && (addr <= 0x07ff)) return chr0400[addr - 0x0400];
	if ((addr >= 0x0800) && (addr <= 0x0bff)) return chr0800[addr - 0x0800];
	if ((addr >= 0x0c00) && (addr <= 0x0fff)) return chr0c00[addr - 0x0c00];
	if ((addr >= 0x1000) && (addr <= 0x13ff)) return chr1000[addr - 0x1000];
	if ((addr >= 0x1400) && (addr <= 0x17ff)) return chr1400[addr - 0x1400];
	if ((addr >= 0x1800) && (addr <= 0x1bff)) return chr1800[addr - 0x1800];
	return chr1c00[addr - 0x1c00];
}

void mmc3_vrom::write(int addr, int addr_from_base, byte data) {
	return;	// rom do nothing.
}

void mmc3_vrom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}