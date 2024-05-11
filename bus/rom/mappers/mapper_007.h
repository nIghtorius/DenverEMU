/*

	Mapper 007 AxROM
	(c) 2023 P. Santing

	Load cart type 0x07 (AMROM/ANROM/AN1ROM/AOROM)

*/

#pragma once

#include "../rom.h"

// machine state AxROM roms
struct axrom_state {
	byte prg = 0;
	byte vrampage = 0;
};

class axrom_rom : public rom {
private:
	byte		*prg8000 = nullptr;
	bus_device	*ppu = nullptr;
	axrom_state state;
public:
	axrom_rom();
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual void	set_rom_data(byte *data, const std::size_t size);
	void			link_ppu_ram(bus_device *ppuram);
	void			update_banks();
};

