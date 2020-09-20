#include "stdafx.h"
#include <malloc.h>
#include <string.h>
#include "bus.h"

#include <iostream>

bus::bus() {
}


bus::~bus() {
}

void	bus::writememory(int addr, byte data) {
	this->address = addr;
	this->write(data);
}

byte	bus::readmemory(int addr) {
	this->address = addr;
	return this->read();
}

word	bus::readmemory_as_word(int addr) {
	word		result;
	this->address = addr;
	result = this->read();
	this->address++;
	return result | this->read() << 8;
}

word	bus::readmemory_as_word_wrap(int addr) {
	word		result;
	this->address = addr;
	result = this->read();
	this->address = (addr & 0xFF00) | ((addr + 1) & 0xFF);
	return result | this->read() << 8;
}

void	bus::write(byte data) {
	for (auto device : this->devices) {
		if ((this->address >= device->devicestart) && (this->address <= device->deviceend)) {
			word caddr = device->compute_addr_from_layout(this->address);
			device->write(caddr, caddr - device->devicestart, data);
		}
	}
}

byte	bus::read() {
	byte	readbus = 0x00;
	for (auto device : this->devices) {
		if ((this->address >= device->devicestart) && (this->address <= device->deviceend)) {
			word caddr = device->compute_addr_from_layout(this->address);
			readbus |= device->read(caddr, caddr - device->devicestart);
		}		
	}
	return readbus;
}

void	bus::registerdevice(bus_device *device) {
	devices.push_back(device);
}

void	bus::removedevice_select_base(int baseaddr) {
	for (size_t i = 0; i < devices.size(); i++) {
		if (devices[i]->devicestart == baseaddr) {
			delete devices[i];	// remove bus device from RAM!
			devices.erase(devices.begin() + i);
			return;
		}
	}
}

bool	bus::nmi_pulled() {
	bool nmi = false;
	for (auto device : devices) {
		if (device->nmi_enable) {
			device->nmi_enable = false;	// NMI is falling edge.
			nmi = true;
		}
	}
	return nmi;
}

bool	bus::irq_pulled() {
	for (auto device : devices) {
		if (device->irq_enable) return true;	// device should pull up IRQ not the interrupted device, therefore only report.
	}
	return false;
}

void	bus::reportdevices() {
	// this reports (dumps to console) all the connected devices with their names and allocation points.
	for (bus_device* dev : devices) {
		std::cout << "registered device [" << dev->get_device_descriptor() << "]" << std::endl;
		std::cout << "-------------------------------------------------------------------------------" << std::endl;
		std::cout << std::hex << "device operating range is [0x" << dev->devicestart << " - 0x" << dev->deviceend << "]" << std::endl;
		std::cout << "device adressing mask is 0x" << dev->devicemask << std::endl;
		std::cout << "device reports size is 0x" << (dev->deviceend - dev->devicestart) << " bytes" << std::endl;
		std::cout << "full bus emulation required? " << (dev->processlayout ? "yes" : "no") << std::endl << std::endl;
	}
}

bus_device::bus_device() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "Denver Base I/O Device");
	this->devicestart = 0x0000;
	this->deviceend = 0xFFFF;
	this->devicemask = 0xFFFF;
	for (int i = 0; i < 16; i++) pinout.pins[i] = i;
}

word bus_device::compute_addr_from_layout(word addr) {
	if (!processlayout) return addr & devicemask;
	word cleanaddr = 0;
	for (int i = 0; i < 16; i++) {
		if (pinout.pins[i] == -1) {
			cleanaddr |= 0x01 << i;	//-1 force signal enable.
		}
		else if (pinout.pins[i] == -2) {
			cleanaddr |= 0x00 << i; //-2 force signal disable.
		} else {
			cleanaddr |= ((addr >> i) & 0x01) << pinout.pins[i];
		}
	}
	return cleanaddr & devicemask;
}

void bus_device::groundpin(int pin) {
	pinout.pins[pin] = -2;
	processlayout = true;
}

void bus_device::vccpin(int pin) {
	pinout.pins[pin] = -1;
	processlayout = true;
}

void bus_device::swappins(int pin1, int pin2) {
	int p = pinout.pins[pin2];
	pinout.pins[pin2] = pinout.pins[pin1];
	pinout.pins[pin1] = p;
	processlayout = true;
}

void bus_device::resetpins_to_default() {
	for (int i = 0; i < 16; i++) pinout.pins[i] = i;
	processlayout = false;
}

bus_device::~bus_device() {
}

char * device::get_device_descriptor() {
	return this->devicedescriptor;
}

void device::dma(byte *data, bool is_output) {
	// dma transfer.
}


void bus_device::write(int addr, int addr_from_base, byte data) {
	
}

byte bus_device::read(int addr, int addr_from_base) {
	return 0x00;
}

device::device() {
	ticksdone = 0;
	tickstodo = 0;
	this->devicedescriptor = (char *)malloc(MAX_DESCRIPTOR_LENGTH);	// reserve 128 bytes.
	strcpy_s(this->devicedescriptor, MAX_DESCRIPTOR_LENGTH, "Denver Base Device");	// default device name for debugging.
}

device::~device() {
	free(this->devicedescriptor);	// be done with it.
}

int device::rundevice(int ticks) {
	return ticks;	// dummy device return same amount of ticks as told to process.
}
