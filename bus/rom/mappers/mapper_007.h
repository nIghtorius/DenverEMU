/*

	Mapper 007 AxROM
	(c) 2023 P. Santing

	Load cart type 0x07 (AMROM/ANROM/AN1ROM/AOROM)

*/

#pragma once

#include "../rom.h"

// machine state AxROM roms
struct axrom_state {
	byte prg;
	byte vrampage;
};

class axrom_rom : public rom {
private:
	byte		*prg8000;
	bus_device	*ppu;
	axrom_state state;
public:
	axrom_rom();
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual void	set_rom_data(byte *data, std::size_t size);
	void			link_ppu_ram(bus_device *ppuram);
	void			update_banks();
};

