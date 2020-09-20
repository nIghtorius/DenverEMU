#include "stdafx.h"
#include "mainram.h"
#include <malloc.h>
#include <iostream>

mainram::mainram() {
	ram = (byte *)malloc(0x0800);	// 2k ram
	devicestart = 0x0000;	// start of device.
	deviceend = 0x1FFF;		// end of device.
	devicemask = 0x07FF;	// mask 2k ( 0000 0111 1111 1111 )	// this enables mirroring for 0x0800, 0x1000 and 0x1800
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver NES mainram (2k)");
	// initialize RAM to 0x00
	memset(ram, 0x00, 0x0800);
}

mainram::~mainram() {
	free(ram);		// dispose ram when disposing memory.
}

void mainram::write(int addr, int addr_from_base, byte data) {
	// addr = real address to write
	// addr_from_base = address always starting relative to zero point. if device is 0x1000 and 0x1002 is written this will be 0x0002
	ram[addr_from_base] = data;	
}

byte mainram::read(int addr, int addr_from_base) {
	// same as write but then read.
	return ram[addr_from_base];
}

// that's it.. al there is needed to emulate system ram
