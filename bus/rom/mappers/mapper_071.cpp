/*

	Camerica mapper implementation.

*/

#include "mapper_071.h"
#include <cstdlib>

#pragma warning (disable : 4996)

// rom

m71rom::m71rom() {
	strncpy(get_device_descriptor(), "Denver Camerica ROM (mapper 071)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
}

void m71rom::setbanks() {
	// banking and mirroring.
	prg_8000 = &romdata[(state.prgbank << 14) % romsize];
	prg_c000 = &romdata[romsize - 16384];

	// mirroring. ToDo::
	if (ppubus == nullptr) return;
	switch (state.mirror) {
	case 0:		
		ppubus->vram.resetpins_to_default();	// normal mode.
		ppubus->vram.groundpin(10);	// force 0x2400-0x27ff -> 0x2000-0x23ff
		break;
	case 1:
		ppubus->vram.resetpins_to_default();	// normal mode.
		ppubus->vram.vccpin(10);	// force 0x2000-0x23ff -> 0x2400-0x27ff
		break;
	}
}

byte m71rom::read(const int addr, const int addr_from_base, const bool onlyread) {
	if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg_8000[addr - 0x8000];
	return prg_c000[addr - 0xC000];
}

void m71rom::write(const int addr, const int addr_from_base, const byte data) {
	if ((addr >= 0x9000) && (addr <= 0x9FFF)) {
		// mirroring
		state.mirror = (data & 0x10) >> 4;
		setbanks();
	}
	if ((addr >= 0xC000) && (addr <= 0xFFFF)) {
		state.prgbank = data & 0x0F;
		setbanks();
	}
}

void m71rom::set_rom_data(byte* data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
	setbanks();
}
