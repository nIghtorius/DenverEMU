/*
	This are the routines that enable working RAM for the nes
	(c) 2018 - P. Santing aka nIghtorius
*/

#pragma once
#include "bus.h"
class mainram :	public bus_device
{
	byte *ram;

public:
	mainram();
	~mainram();
	void	write(int addr, int addr_from_base, byte data);
	byte	read(int addr, int addr_from_base);
};

