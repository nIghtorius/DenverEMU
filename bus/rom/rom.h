/*
	base cartridge rom
	(c) 2018 P. Santing.

	load cart type 0x00 (normal ROM)

	// rom specifics.
	// load from 0x8000 a 16kB or 32kB block
	// when loading 16kB block. devicemask will mask so that first 16kB will be mirrored.

	// vrom specifics
	// load char rom up to 8kB
*/

#pragma once

#include "../bus.h"
class vrom : public bus_device {
private:
public:
	vrom();
	~vrom();
	virtual void	set_rom_data(byte *data, std::size_t size);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
	void			link_ppu_bus(bus_device *ppu_bus);
	bus_device		*ppubus;
	byte			*romdata;
	int				romsize;
};

class vram : public vrom {
private:
	byte	*ram;
public:
	vram();
	~vram();
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual byte	read(int addr, int addr_from_base, bool onlyread = false);
	void			reset();
};

class rom : public bus_device {
private:
public:
	rom();
	~rom();
	virtual	void	set_rom_data(byte *data, std::size_t size);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual	byte	read(int addr, int addr_from_base, bool onlyread = false);
	byte	*romdata;
	int		romsize;
};
