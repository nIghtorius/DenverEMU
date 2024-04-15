/*

	Mapper 071 Camerica.
	(c) 2024 P. Santing

*/

#pragma once

#include "../rom.h"

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
	m71rom();
	virtual byte read(int addr, int addr_from_base, bool onlyread = false);
	virtual void write(int addr, int addr_from_base, byte data);
	virtual void set_rom_data(byte* data, std::size_t size);
};

