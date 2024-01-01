/*

	AxROM implementation

*/

#pragma warning(disable : 4996)

#include "mapper_007.h"

axrom_rom::axrom_rom() {
	strncpy(get_device_descriptor(), "Denver AxROM ROM (Mapper 007)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
}

void axrom_rom::set_rom_data(byte *data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
}

void  axrom_rom::link_ppu_ram(bus_device *ppuram) {
	ppu = ppuram;
}

void axrom_rom::update_banks() {
	prg8000 = &romdata[(state.prg << 15) % romsize];
	if (ppu) {
		if (state.vrampage == 0) {
			ppu->resetpins_to_default();
			ppu->groundpin(10);				// 1 - screen NT A
		}
		else {
			ppu->resetpins_to_default();
			ppu->vccpin(10);				// 1 - screen NT B
		}
	}
}

byte axrom_rom::read(int addr, int addr_from_base, bool onlyread) {
	return prg8000[addr_from_base];
}

void axrom_rom::write(int addr, int addr_from_base, byte data) {
	state.prg = data & 0x07;
	state.vrampage = (data & 0x10) >> 4;
	update_banks();
}
