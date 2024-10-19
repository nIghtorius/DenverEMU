#include "mapper_004.h"
#include "../../video/ppu.h"
#include <iostream>
#include "../../package/2a03.h"

#pragma warning(disable : 4996)

// implementation of MMC3 (mapper_004.h)

// rom side.
mmc3_rom::mmc3_rom() {
	strncpy(get_device_descriptor(), "Denver MMC3 (mapper 004)", MAX_DESCRIPTOR_LENGTH);
	prgram6000 = (byte *)malloc(8192);
	set_debug_data();
}

mmc3_rom::~mmc3_rom() {
	free(prgram6000);
}

void mmc3_rom::reset() {
	if (state.mapper_52_mode) state.ob_register = 0x00; write(0x6000, 0x0000, 0x00);
	if (state.mapper_44_mode) state.ob_register = 0x00; write(0xa001, 0x4001, 0x00);
}

batterybackedram* mmc3_rom::get_battery_backed_ram() {
	return new batterybackedram((byte*)prgram6000, 8192);
}

void mmc3_rom::set_battery_backed_ram(byte* data, const std::size_t size) {
	if (size > 8192) return;
	memcpy(prgram6000, data, size);
}

byte	mmc3_rom::read(const int addr, const int addr_from_base, const bool onlyread)
{
	if ((addr >= 0x6000) && (addr <= 0x7fff)) {
		return prgram6000[addr - 0x6000];
		//return state.prg_ram_enable ? prgram6000[addr - 0x6000] : 0;
	}
	if ((addr >= 0x8000) && (addr <= 0x9fff)) return prg8000[addr - 0x8000];
	if ((addr >= 0xa000) && (addr <= 0xbfff)) return prga000[addr - 0xa000];
	if ((addr >= 0xc000) && (addr <= 0xdfff)) return prgc000[addr - 0xc000];
	return prge000[addr - 0xe000];
}

void	mmc3_rom::write(const int addr, const int addr_from_base, const byte data) {
	if (state.mapper_52_mode && !(state.ob_register & MP52_LOCK_UNTIL_RESET) && (addr <= 0x7FFF)) {
		state.ob_register = data;
		state.prgand = 0x1f;
		if (data & MP52_NOPRG256KB) state.prgand = 0x0F;
		state.chrand = 0xff;
		if (data & MP52_NOCHR256KB) state.chrand = 0x7F;
		state.prgor = (data & 0x07) << 4;
		state.chror = ((data >> 3) & 0x04) | ((data >> 1) & 0x02) | ((data >> 6) & (data >> 4) & 0x01);
		state.chror <<= 0x07;
		if (!(data & MP52_NOPRG256KB)) state.prgor &= 0x60;
		if (!(data & MP52_NOCHR256KB)) state.chror &= ~0xFF;
		update_banks();
		return;
	}
	if ((addr == 0xA001) && state.mapper_44_mode) {
		// mapper 44 registers.
		byte block = data & 0x07;
		switch (block) {
		case 0x00:
			state.prgand = 0x0F;
			state.prgor = 0x00;
			state.chrand = 0x7F;
			state.chror = 0x0000;
			break;
		case 0x01:
			state.prgand = 0x0F;
			state.prgor = 0x10;
			state.chrand = 0x7F;
			state.chror = 0x0080;
			break;
		case 0x02:
			state.prgand = 0x0F;
			state.prgor = 0x20;
			state.chrand = 0x7F;
			state.chror = 0x0100;
			break;
		case 0x03:
			state.prgand = 0x0F;
			state.prgor = 0x30;
			state.chrand = 0x7F;
			state.chror = 0x0180;
			break;
		case 0x04:
			state.prgand = 0x0F;
			state.prgor = 0x40;
			state.chrand = 0x7F;
			state.chror = 0x0200;
			break;
		case 0x05:
			state.prgand = 0x0F;
			state.prgor = 0x50;
			state.chrand = 0x7F;
			state.chror = 0x0280;
			break;
		case 0x06:
		case 0x07:
			state.prgand = 0x1F;
			state.prgor = 0x60;
			state.chrand = 0xFF;
			state.chror = 0x0300;
			break;
		}
		update_banks();
	}
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
			// mirroring.
			if (state.do_horizontal_mirroring) {
				vbus->vram.resetpins_to_default();
				vbus->vram.swappins(10, 11);
				vbus->vram.groundpin(10);
			}
			else {
				vbus->vram.resetpins_to_default();
			}
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

void	mmc3_rom::write_banks(const byte data) {
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
	byte sndlastbank = (romsize - 16384) / 8192;
	byte lastbank = (romsize - 8192) / 8192;

	// do mapper 52 stuff.
	if (state.mapper_52_mode || state.mapper_44_mode) {
		// recompute lastbank.
		sndlastbank = (sndlastbank & state.prgand) | state.prgor;
		lastbank = (lastbank & state.prgand) | state.prgor;
	}
	else {
		state.prgand = 0xff;
		state.chrand = 0xff;
		state.prgor = 0x00;
		state.chror = 0x00;
	}

	if (!state.prg_bank_mode) {
		// R6 => 0x8000-0x9FFF
		prg8000 = &romdata[(((state.r6 & state.prgand) | state.prgor) << 13) % romsize];
		prgc000 = &romdata[(sndlastbank << 13) % romsize];
	}
	else {
		// R6 => 0xC000-0xDFFF
		prgc000 = &romdata[(((state.r6 & state.prgand) | state.prgor) << 13) % romsize];
		prg8000 = &romdata[(sndlastbank << 13) % romsize];
	}
	// register 7
	prga000 = &romdata[(((state.r7 & state.prgand) | state.prgor) << 13) % romsize];
	prge000 = &romdata[(lastbank << 13) % romsize];

	// trigger the event to mmc3_vrom
	if (vrom) vrom->update_banks(&state);
}

void	mmc3_rom::set_mapper52_mode() {
	strncpy(get_device_descriptor(), "Denver MMC3 - Extended (mapper 052)", MAX_DESCRIPTOR_LENGTH);
	state.mapper_52_mode = true;
	// write initial state.
	write(0x6000, 0x0000, 0x00);
}

void	mmc3_rom::set_mapper44_mode() {
	strncpy(get_device_descriptor(), "Denver MMC3 - Extended (mapper 044)", MAX_DESCRIPTOR_LENGTH);
	state.mapper_44_mode = true;
	// write initial state.
	write(0xA001, 0x4001, 0x00);
}

void	mmc3_rom::set_rom_data(byte *data, const std::size_t size) {
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

int		mmc3_rom::rundevice(const int ticks) {
	cpu_tk += ticks;
	if (!vrom) return ticks;
	word ppuaddr = vbus->vbus.address;
	bool a12_risen = ((ppuaddr & 0x1000) > 0) && ((lastppuaddr & 0x1000) == 0);	
	if ((a12_risen) && (cpu_tk - lt_a12r >= 9)) {
		// log ppu scanline or a12 rise.	
		/*
		int scanline = reinterpret_cast<ppu*>(devicebus->devices[2])->scanline;
		int ppucycle = reinterpret_cast<ppu*>(devicebus->devices[2])->cycle;
		int cpux = reinterpret_cast<package_2a03*>(devicebus->devices[0])->cpu_2a03.regs.x;
		std::cout << "A12 rise on scanline #" << std::dec << scanline << " address is @ " << std::hex << (int)ppuaddr;
		std::cout << " last address is @ " << (int)lastppuaddr << " ppu cycle: " << std::dec << ppucycle << " cpu.x: " << cpux << "\n";
		*/
		lt_a12r = cpu_tk;
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

void mmc3_rom::set_debug_data() {
	debugger.add_debug_var("MMC3", -1, NULL, t_beginblock);
	debugger.add_debug_var("CPU ticks @ ROM", -1, &cpu_tk, t_int);
	debugger.add_debug_var("CPU tick value last A12 rise", -1, &lt_a12r, t_int);
	debugger.add_debug_var("ROM Size", -1, &romsize, t_int);
	debugger.add_debug_var("Bank update register", -1, &state.bank_update_reg, t_byte);
	debugger.add_debug_var("PRG bank mode", -1, &state.prg_bank_mode, t_byte);
	debugger.add_debug_var("CHAR a12 inverted", -1, &state.chr_a12_inv, t_bool);
	debugger.add_debug_var("Horizontal mirroring", -1, &state.do_horizontal_mirroring, t_bool);
	debugger.add_debug_var("PRG RAM Enable", -1, &state.prg_ram_enable, t_bool);
	debugger.add_debug_var("PRG RAM RO", -1, &state.prg_ram_readonly, t_bool);
	debugger.add_debug_var("IRQ Latch", -1, &state.irq_latch, t_byte);
	debugger.add_debug_var("IRQ Counter", -1, &state.irq_counter, t_byte);
	debugger.add_debug_var("IRQ Enabled", -1, &state.irq_enable, t_bool);
	debugger.add_debug_var("IRQ Reload", -1, &state.irq_reload, t_bool);
	word* registers = &state.r0;
	debugger.add_debug_var("MMC3 registers", 8, registers, t_shortintarray);
	debugger.add_debug_var("MMC3", -1, NULL, t_endblock);
}

mmc3_vrom::mmc3_vrom() {
	strncpy(get_device_descriptor(), "Denver MMC3 VROM (mapper 004)", MAX_DESCRIPTOR_LENGTH);
	trigger_a_change = false;
}

word mmc3_vrom::fetch_ppu_addr() {
	return 0;// ppuaddr;
}

void mmc3_vrom::update_banks(mmc3_state *state) {
	if (!romdata) return;
	if (!state->chr_a12_inv) {
		chr0000 = &romdata[(( ((state->r0 & state->chrand) | state->chror) & 0xFFFE) << 10) % romsize];
		chr0400 = &romdata[(((((state->r0 & state->chrand) | state->chror) & 0xFFFE) << 10) + 1024) % romsize];
		chr0800 = &romdata[((((state->r1 & state->chrand) | state->chror) & 0xFFFE) << 10) % romsize];
		chr0c00 = &romdata[(((((state->r1 & state->chrand) | state->chror) & 0xFFFE) << 10) + 1024) % romsize];
		chr1000 = &romdata[(((state->r2 & state->chrand) | state->chror) << 10)%romsize];
		chr1400 = &romdata[(((state->r3 & state->chrand) | state->chror) << 10)%romsize];
		chr1800 = &romdata[(((state->r4 & state->chrand) | state->chror) << 10)%romsize];
		chr1c00 = &romdata[(((state->r5 & state->chrand) | state->chror) << 10)%romsize];
	}
	else {
		chr0000 = &romdata[(((state->r2 & state->chrand) | state->chror) << 10)%romsize];
		chr0400 = &romdata[(((state->r3 & state->chrand) | state->chror) << 10)%romsize];
		chr0800 = &romdata[(((state->r4 & state->chrand) | state->chror) << 10)%romsize];
		chr0c00 = &romdata[(((state->r5 & state->chrand) | state->chror) << 10)%romsize];
		chr1000 = &romdata[((((state->r0 & state->chrand) | state->chror) & 0xFFFE) << 10) % romsize];
		chr1400 = &romdata[(((((state->r0 & state->chrand) | state->chror) & 0xFFFE) << 10) + 1024) % romsize];
		chr1800 = &romdata[((((state->r1 & state->chrand) | state->chror) & 0xFFFE) << 10) % romsize];
		chr1c00 = &romdata[(((((state->r1 & state->chrand) | state->chror) & 0xFFFE) << 10) + 1024) % romsize];
	}
}

byte mmc3_vrom::read(const int addr, const int addr_from_base, const bool onlyread) {
	//if (!onlyread) ppuaddr = addr;
	if (!chr0000) return 0;	// not initialized yet.
	if ((addr >= 0x0000) && (addr <= 0x03ff)) return chr0000[addr];
	if ((addr >= 0x0400) && (addr <= 0x07ff)) return chr0400[addr - 0x0400];
	if ((addr >= 0x0800) && (addr <= 0x0bff)) return chr0800[addr - 0x0800];
	if ((addr >= 0x0c00) && (addr <= 0x0fff)) return chr0c00[addr - 0x0c00];
	if ((addr >= 0x1000) && (addr <= 0x13ff)) return chr1000[addr - 0x1000];
	if ((addr >= 0x1400) && (addr <= 0x17ff)) return chr1400[addr - 0x1400];
	if ((addr >= 0x1800) && (addr <= 0x1bff)) return chr1800[addr - 0x1800];
	return chr1c00[addr - 0x1c00];	//if (addr <= 0x1fff) 
}

void mmc3_vrom::write(const int addr, const int addr_from_base, const byte data) {
	//ppuaddr = addr;
	return;	// rom do nothing.
}

void mmc3_vrom::set_rom_data(byte *data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
	devicestart = 0x0000;
	deviceend = 0x3FFF;
	devicemask = 0x3FFF;	// expand device to max.
}