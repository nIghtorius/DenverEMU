#include "debug6k.h"
#include <iostream>

#pragma warning(disable : 4996)

debug6k::debug6k() {
	devicestart = 0x6000;	// start of device.
	deviceend = 0x6FFF;		// end of device.
	devicemask = 0x6FFF;	
	strncpy(get_device_descriptor(), "Denver blargg 0x6000 debugger", MAX_DESCRIPTOR_LENGTH);
}


debug6k::~debug6k() {
}

void debug6k::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base > 3)  std::cout << data;
}

bogusdevice::bogusdevice() {
	strncpy(get_device_descriptor(), "Device that does 2 clocks per clock request", MAX_DESCRIPTOR_LENGTH);
	tick_rate = 2;
}

bogusdevice::~bogusdevice() {

}

int	 bogusdevice::rundevice (int ticks) {
	return (ticks / 2) * 2;
}