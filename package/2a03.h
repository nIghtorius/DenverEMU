/*

	2a03 package (package_2a03)
	Packages virtual devices cpu2a03_fast, apu and controllers into a singular
	virtual device.

*/

#pragma once

#include "../bus/bus.h"
#include "../cpu/cpu2a03_fast.h"
#include "../audio/apu.h"
#include "../controller/joypad.h"

#define CPU_DMA					0x14

class package_2a03 : public bus_device {
public:
	// localized devices.
	apu					apu_2a03;
	cpu2a03_fast		cpu_2a03;
	nes_2a03_joyports	controllers;
	bool				no_apu = false;

	package_2a03();

	byte	read(int addr, int addr_from_base, bool onlyread = false);
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
	void	set_joydefs(joypad * joydefs);
	void	reset();
	void	dma(byte *data, bool is_output, bool started);
	virtual void	_attach_to_bus(bus * attachedbus);
};
