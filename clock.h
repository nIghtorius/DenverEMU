/*

	clock.h
	(c) 2018 P. Santing
	for the Denver Project.

	what does it do:
	It manages the clock ratio's between devices.. such as PPU's and CPU's, etc..
	
*/

#pragma once

#include "bus.h"
#include <vector>

class clock
{
private:
		std::vector<device *>devices;
public:
	clock();
	~clock();
	void registerdevice(device *adevice);
	void step();
	void run();
};

// faster variant. static clocks.
class fastclock {
private:
	device *cpudevice;
	device *ppudevice;
	device *apudevice;
public:
	fastclock();
	~fastclock();
	void step();
	void run();
	void setdevices(device *cpu, device *ppu, device *apu);
};
