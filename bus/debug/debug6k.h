#pragma once
#include "..\..\bus\bus.h"

// debugger for blargg test roms (singles)

class debug6k :
	public bus_device
{
public:
	debug6k();
	~debug6k();
	void	write(int addr, int addr_from_base, byte data);
};

class bogusdevice : public device {
public:
	bogusdevice();
	~bogusdevice();
	int	rundevice(int ticks);
};