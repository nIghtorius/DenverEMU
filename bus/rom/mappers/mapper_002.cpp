#include "stdafx.h"
#include "mapper_002.h"

// implementation of UxROM (mapper_002.h)

uxrom::uxrom() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver UxROM (mapper 002)");
}

byte uxrom::read(int addr, int addr_from_base)
{
	return addr_from_base >= 0x4000 ? 
		romdata[(addr & 0x3FFF) | lastbank] : 
		romdata[(bank << 14) | addr_from_base];
}

void uxrom::write(int addr, int addr_from_base, byte data)
{
	// we just look @ data, address is irrelevant.
	bank = data & 0x07;	// last 3 bits.
}

void uxrom::set_rom_data(byte *data, size_t size)
{
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;
	lastbank = romsize - 16384;
}