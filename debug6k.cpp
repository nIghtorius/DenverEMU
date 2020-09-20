#include "stdafx.h"
#include "debug6k.h"
#include <iostream>

debug6k::debug6k() {
	devicestart = 0x6000;	// start of device.
	deviceend = 0x6FFF;		// end of device.
	devicemask = 0x6FFF;	
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver blargg 0x6000 debugger");
}


debug6k::~debug6k() {
}

void debug6k::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base > 3)  std::cout << data;
}

bogusdevice::bogusdevice() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Device that does 2 clocks per clock request");
	tick_rate = 2;
}

bogusdevice::~bogusdevice() {

}

int	 bogusdevice::rundevice (int ticks) {
	return (ticks / 2) * 2;
}