#include "rom.h"

#pragma warning(disable : 4996)


rom::rom() {
	// default to 16K rom with mirrored 0xC000 (to 0x8000)
	strncpy(this->get_device_descriptor(), "NROM 0x00 ROM", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x8000;
	deviceend = 0xFFFF;
	devicemask = 0xBFFF;	// 1011 1111 1111 1111
	// create space for SRAM file name.
	sramfile = (char*)malloc(SRAM_MAX_FILE_NAME);
	if (sramfile != nullptr)
		strncpy(sramfile, "", SRAM_MAX_FILE_NAME);
}

rom::~rom() {
}

void rom::set_rom_data(byte *data, const std::size_t size) {
	this->romdata = data;
	this->romsize = (int)size;
	// extra logic for 16kB / 32kB roms.
	if (size == 16384) devicemask = 0xBFFF; else devicemask = 0xFFFF;
}

char* rom::get_sram_filename() {
	return sramfile;
}

batterybackedram* rom::get_battery_backed_ram() {
	return NULL; // does not have battery packed ram.
}

void rom::set_battery_backed_ram(byte* data, const std::size_t size) {
	// do nothing.
}

void rom::write(const int addr, const int addr_from_base, const byte data) {
	// do actually nothing.
	// ROM are read only and cannot be written to.
	// especially ROM 0x00 -> other roms with mappers will control the mapper logic.
}

byte rom::read(const int addr, const int addr_from_base, const bool onlyread) {
	// read ROM data.
	return romdata[addr_from_base];
}

vrom::vrom() {
	strncpy(this->get_device_descriptor(), "NROM 0x00 VROM", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1FFF;
	devicemask = 0x1FFF;
}

vrom::~vrom() {
}

void vrom::set_rom_data(byte *data, const std::size_t size) {
	this->romdata = data;
	this->romsize = (int)size;
	devicemask = ((int)size - 1);
}

void vrom::write(const int addr, const int addr_from_base, const byte data) {
	// do nothing ROM=ROM as in read ONLY memory
}

byte vrom::read(const int addr, const int addr_from_base, const bool onlyread) {
	return romdata[addr_from_base];
}

void vrom::link_ppu_bus(bus_device *ppu_bus) {
	ppubus = ppu_bus;
}

vram::vram() {
	strncpy(this->get_device_descriptor(), "NROM 0x00 VRAM", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0x1fff;
	devicemask = 0x1fff;
	ram = (byte *)malloc(8192);
}

vram::~vram() {
	free(ram);
}

void vram::write(const int addr, const int addr_from_base, const byte data) {
	ram[addr_from_base] = data;
}

byte vram::read(const int addr, const int addr_from_base, const bool onlyread) {
	return ram[addr_from_base];
}

void vram::reset() {
	memset(ram, 0, 8192);
}