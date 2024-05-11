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

#define		SRAM_MAX_FILE_NAME 512

struct batterybackedram {
	byte*		data = nullptr;
	std::size_t	size = 0;
	constexpr batterybackedram(byte* _data, int _size) : data(_data), size(_size) {}
};

class vrom : public bus_device {
private:
public:
	vrom();
	~vrom();
	virtual void	set_rom_data(byte *data, const std::size_t size);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	void			link_ppu_bus(bus_device *ppu_bus);
	bus_device		*ppubus = nullptr;
	byte			*romdata = nullptr;
	int				romsize = 0;
};

class vram : public vrom {
private:
	byte	*ram;
public:
	vram();
	~vram();
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	void			reset();
};

class rom : public bus_device {
private:
	char* sramfile = nullptr;
public:
	rom();
	~rom();
	virtual	void	set_rom_data(byte *data, const std::size_t size);
	virtual void	write(const int addr, const int addr_from_base, const byte data);
	virtual	byte	read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual batterybackedram* get_battery_backed_ram();
	virtual void	set_battery_backed_ram(byte* data, const std::size_t size);
	char* get_sram_filename();
	byte	*romdata = nullptr;
	int		romsize = 0;
	bool	battery = false;
};
