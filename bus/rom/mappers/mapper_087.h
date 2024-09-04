/*

	Mapper 087 J87
	(c) 2024 P. Santing

	Load cart type 87 (J87)

	// mapper specifics.
	// max 32KB PRG (NROM)

	// write to ROM area changes CHR-ROM
	// filter last 2 bits, for max 4x 8KB=32KB chrrom

*/

#pragma once

#include "../rom.h"

class j87vrom : public vrom {
private:
	byte* charbank = nullptr;

public:
	j87vrom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void set_rom_data(byte* data, const std::size_t size);
	void setbank(const int bank);
};

class j87rom : public rom {
private:
	j87vrom* charrom = nullptr;
public:
	virtual void write(const int addr, const int addr_from_base, const byte data);
	void link_vrom(j87vrom* rom);
};