#include "mapper_087.h"

#pragma warning(disable: 4996)

// implementation of J87ROM

j87vrom::j87vrom() {
	strncpy(get_device_descriptor(), "Denver J87 VROM (mapper 087)", MAX_DESCRIPTOR_LENGTH);
}

byte j87vrom::read(const int addr, const int addr_from_base, const bool onlyread) {
	return charbank[addr];
}

void j87vrom::setbank(const int bank) {
	charbank = &romdata[(bank << 13) % romsize];
}

void j87vrom::set_rom_data(byte* data, const std::size_t size) {
	devicestart = 0x0000;
	deviceend = 0x1fff;
	devicemask = 0x1fff;
	romdata = data;
	romsize = (int)size;
	charbank = &romdata[0];
}

void j87rom::write(const int addr, const int addr_from_base, const byte data) {
	if (charrom) {
		charrom->setbank(data & 0x03);
	}
}

void j87rom::link_vrom(j87vrom* rom) {
	charrom = rom;
}
