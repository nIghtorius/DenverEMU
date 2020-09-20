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

#include "bus.h"

class vrom : public bus_device {
	byte	*romdata;
	int		romsize;
public:
	vrom();
	~vrom();
	virtual void	set_rom_data(byte *data, int size);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual byte	read(int addr, int addr_from_base);
};

class vram : public bus_device {
	byte	*ram;
	int		ramsize;
public:
	vram();
	~vram();
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual byte	read(int addr, int addr_from_base);
};

class rom : public bus_device {
	byte	*romdata;
	int		romsize;

public:
	rom();
	~rom();
	virtual	void	set_rom_data(byte *data, int size);
	virtual void	write(int addr, int addr_from_base, byte data);
	virtual	byte	read(int addr, int addr_from_base);
};

// cartridge class ( contains both ROM and VROM )
class cartridge {
private:
	bus_device	*romdevice;
	bus_device	*vromdevice;
public:
	cartridge(bus_device *rd, bus_device *vrd);
	~cartridge();
};
