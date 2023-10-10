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
	virtual	byte read(int addr, int addr_from_base);
	virtual void set_rom_data(byte *data, std::size_t size);
	void	setbank(int bank);
};

class cnrom : public rom {
private:
	cnvrom	*charrom = nullptr;
public:
	virtual void write(int addr, int addr_from_base, byte data);
	void	link_vrom(cnvrom *rom);
};