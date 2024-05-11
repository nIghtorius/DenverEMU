/*

	Mapper 003 CNROM
	(c) 2023 P. Santing

	Load cart type 0x03 (CNROM)

	// mapper specifics.
	// max 32KB PRG (NROM)

	// write to ROM area changes CHR-ROM
	// filter last 2 bits, for max 4x 8KB=32KB chrrom

*/

#pragma once

#include "../rom.h"

class cnvrom : public vrom {
private:
	byte	*charbank;

public:
	cnvrom();
	virtual	byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void set_rom_data(byte *data, const std::size_t size);
	void	setbank(const int bank);
};

class cnrom : public rom {
private:
	cnvrom	*charrom = nullptr;
public:
	virtual void write(const int addr, const int addr_from_base, const byte data);
	void	link_vrom(cnvrom *rom);
};