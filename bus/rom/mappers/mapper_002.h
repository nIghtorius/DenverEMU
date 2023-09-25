/*

	Mapper 002 UxROM
	(c) 2023 P. Santing

	Load cart type 0x02 (UxROM)

	// rom specifics.
	// load state 0x8000 = Switchable, defaults to first.
	//			  0xC000 = Fixed, Last 16kB block of ROM data.

	// Back is selected by writing to 0x8000-0xFFFF
	// 7 bit  0
	// --------
	// xxxxpPPP		- UxROM uses PPP, UOROM uses pPPP

	// vrom specifics
	// No VROM has VRAM

*/

#pragma once

#include "../rom.h"		// also includes the basic 8kB vram class.

class uxrom : public rom {
private:
	byte	bank;
	int		lastbank;
public:
	uxrom();
	virtual byte	read(int addr, int addr_from_base);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual void	set_rom_data(byte *data, size_t size);
};
