/*

	Camerica mapper implementation.

*/

#include "mapper_071.h"
#include <cstdlib>

#pragma warning (disable : 4996)

// rom

m71rom::m71rom() {
	strncpy(get_device_descriptor(), "Denver Camerica Rom (mapper 071)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
}

void m71rom::setbanks() {
	// banking and mirroring.
	prg_8000 = &romdata[(state.prgbank << 14) % romsize];
	prg_c000 = &romdata[romsize - 16384];

	// mirroring. ToDo::
	switch (state.mirror) {
	case 0:		
		break;
	case 1:
		break;
	}
}

byte m71rom::read(int addr, int addr_from_base, bool onlyread) {
	if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg_8000[addr - 0x8000];
	return prg_c000[addr - 0xC000];
}

void m71rom::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0x8000) && (addr <= 0x9FFF)) {
		// mirroring
		state.mirror = (data & 0x10) >> 4;
		setbanks();
	}
	if ((addr >= 0xC000) && (addr <= 0xFFFF)) {
		state.prgbank = data & 0x0F;
		setbanks();
	}
}

void m71rom::set_rom_data(byte* data, std::size_t size) {
	romdata = data;
	romsize = (int)size;
	setbanks();
}
