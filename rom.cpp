#include "stdafx.h"
#include "rom.h"

rom::rom() {
	// default to 16K rom with mirrored 0xC000 (to 0x8000)
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "NROM 0x00 ROM");
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xBFFF;	// 1011 1111 1111 1111
}

rom::~rom() {
}

void rom::set_rom_data(byte *data, int size) {
	this->romdata = data;
	this->romsize = size;
	// extra logic for 16kB / 32kB roms.
	if (size == 16384) devicemask = 0xBFFF; else devicemask = 0xFFFF;
}

void rom::write(int addr, int addr_from_base, byte data) {
	// do actually nothing.
	// ROM are read only and cannot be written to.
	// especially ROM 0x00 -> other roms with mappers will control the mapper logic.
}

byte rom::read(int addr, int addr_from_base) {
	// read ROM data.
	return romdata[addr_from_base];
}

vrom::vrom() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "NROM 0x00 VROM");
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

vrom::~vrom() {
}

void vrom::set_rom_data(byte *data, int size) {
	this->romdata = data;
	this->romsize = size;
	devicemask = (size - 1);
}

void vrom::write(int addr, int addr_from_base, byte data) {
	// do nothing ROM=ROM as in read ONLY memory
}

byte vrom::read(int addr, int addr_from_base) {
	return romdata[addr_from_base];
}

vram::vram() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "NROM 0x00 VRAM");
	devicestart = 0x0000;
	deviceend = 0x1fff;
	devicemask = 0x1fff;
	ram = (byte *)malloc(8192);
}

vram::~vram() {
	free(ram);
}

void vram::write(int addr, int addr_from_base, byte data) {
	ram[addr_from_base] = data;
}

byte vram::read(int addr, int addr_from_base) {
	return ram[addr_from_base];
}

cartridge::cartridge(bus_device *rd, bus_device *vrd) {
	this->romdevice = rd;
	this->vromdevice = vrd;
}

cartridge::~cartridge() {
	// destroy roms
	delete romdevice;
	delete vromdevice;
}