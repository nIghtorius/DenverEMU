#include "mapper_003.h"
#include <iostream>

// implementation of CNROM (mapper_003.h)

cnvrom::cnvrom() {
	strcpy_s(get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver CNROM (mapper 003)");
}

byte	cnvrom::read(int addr, int addr_from_base) {
	return charbank[addr];
}

void	cnvrom::setbank(int bank) {
	charbank = &romdata[(bank << 13) % romsize];
}

void	cnvrom::set_rom_data(byte *data, std::size_t size) {
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
	romdata = data;
	romsize = (int)size;
	charbank = &romdata[0];
}

void	cnrom::write(int addr, int addr_from_base, byte data) {
	if (charrom) {
		charrom->setbank(data & 0x03);
	}
}

void	cnrom::link_vrom(cnvrom *rom) {
	charrom = rom;
}
