/*

	Mapper 071 Camerica.
	(c) 2024 P. Santing

*/

#pragma once

#include "../rom.h"
#include "../../../video/ppu.h"

struct m71_state {
	byte outerbank = 0;	// needs work.
	byte prgbank = 0;
	byte mirror = 0;
};

class m71rom : public rom {
private:
	m71_state state;
	byte* prg_8000 = nullptr;
	byte* prg_c000 = nullptr;
	void setbanks();
public:
	ppu* ppubus = nullptr;
	m71rom();
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual void set_rom_data(byte* data, const std::size_t size);
};

