#include "mapper_002.h"

#pragma warning(disable : 4996)

// implementation of UxROM (mapper_002.h)

uxrom::uxrom() {
	strncpy(get_device_descriptor(), "Denver UxROM (mapper 002)", MAX_DESCRIPTOR_LENGTH);
}

byte uxrom::read(int addr, int addr_from_base)
{
	return addr_from_base >= 0x4000 ? 
		romdata[(addr & 0x3FFF) | lastbank] : 
		romdata[(bank << 14) %romsize | addr_from_base];
}

void uxrom::write(int addr, int addr_from_base, byte data)
{
	// we just look @ data, address is irrelevant.
	bank = data & 0x07;	// last 3 bits.
}

void uxrom::set_rom_data(byte *data, std::size_t size)
{
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	romdata = data;
	romsize = (int)size;
	lastbank = romsize - 16384;
}
