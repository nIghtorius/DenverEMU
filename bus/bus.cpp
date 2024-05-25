#include <cstdlib>
#include <string>
#include "bus.h"
#include <iostream>

#pragma warning(disable : 4996)

bus::bus() {
}

bus::~bus() {
	// free all connected devices.
	for (auto device : devices) delete device;
}

void	bus::writememory(const int addr, const byte data) {
	address = addr;
	write(data);
}

byte	bus::readmemory(const int addr, const bool onlyread) {
	address = addr;
	return read();
}

word	bus::readmemory_as_word(const int addr, const bool onlyread) {
	word		result;
	address = addr;
	result = read();
	address++;
	return result | read() << 8;
}

word	bus::readmemory_as_word_wrap(const int addr, const bool onlyread) {
	word		result;
	address = addr;
	result = read();
	address = (addr & 0xFF00) | ((addr + 1) & 0xFF);
	return result | read() << 8;
}

void	bus::write(const byte data) {
	for (auto device : devices) {
		if ((address >= device->devicestart) && (address <= device->deviceend)) {
			word caddr = device->compute_addr_from_layout(address);
			device->write(caddr, caddr - device->devicestart, data);
			if (!no_bus_conflicts) break;	// do not emulate bus conflicts, we are done.
		}
	}
}

byte	bus::read(const bool onlyread) {
	byte	readbus = 0x00;
	for (auto device : devices) {
		if ((address >= device->devicestart) && (address <= device->deviceend)) {
			word caddr = device->compute_addr_from_layout(address);
			readbus |= device->read(caddr, caddr - device->devicestart, onlyread);
			if (!no_bus_conflicts) break; // do not emulate bus conflicts, we are done.
		}		
	}
	return readbus;
}

void	bus::emulate_bus_conflicts(const bool enable) {
	no_bus_conflicts = enable;
}

void	bus::registerdevice(bus_device *device) {
	if (device) {
		devices.push_back(device);
		device->_attach_to_bus(this);
	}
}

void	bus::removedevice_select_base(const int baseaddr) {
	for (std::size_t i = 0; i < devices.size(); i++) {
		if (devices[i]->devicestart == baseaddr) {
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

void	bus::busreset() {
	for (auto device : devices) {
		device->reset();
	}
}

void	bus::reportdevices() {
	// this reports (dumps to console) all the connected devices with their names and allocation points.
	for (bus_device* dev : devices) {
		std::cout << "[" << dev->get_device_descriptor() << "] ";
		std::cout << std::hex << "[0x" << dev->devicestart << " - 0x" << dev->deviceend << "]";
		std::cout << " [MASK 0x" << dev->devicemask << "] ";
		std::cout << " [SIZE 0x" << (dev->deviceend - dev->devicestart) << "] [FB:";
		std::cout << (dev->processlayout ? "yes" : "no") << "]" << std::endl;
	}
}

bus_device::bus_device() {
	strncpy(get_device_descriptor(), "Denver Base I/O Device", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	devicebus = nullptr;
	for (int i = 0; i < 16; i++) pinout.pins[i] = i;
	set_debug_data();
}

void bus_device::set_debug_data() {
	// placeholder debug data.
	debugger.add_debug_var("Device Bus", -1, NULL, t_beginblock);
	debugger.add_debug_var("Need to emulate pinlayout", -1, &processlayout, t_bool);
	debugger.add_debug_var("Device starts at", -1, &devicestart, t_addr);
	debugger.add_debug_var("Device ends at", -1, &deviceend, t_addr);
	debugger.add_debug_var("Device mask", -1, &devicemask, t_addr);
	debugger.add_debug_var("Bus Layout", 16, &pinout.pins[0], t_shortintarray);
	debugger.add_debug_var("Device Bus", -1, NULL, t_endblock);
	debugger.add_debug_var("Device Specific", -1, NULL, t_beginblock);
	debugger.add_debug_var("IRQ asserted", -1, &irq_enable, t_bool);
	debugger.add_debug_var("NMI asserted", -1, &nmi_enable, t_bool);
	debugger.add_debug_var("Processing DMA", -1, &in_dma_mode, t_bool);
	debugger.add_debug_var("DMA needs to start", -1, &dma_start, t_bool);
	debugger.add_debug_var("Device Specific", -1, NULL, t_endblock); 
}

word bus_device::compute_addr_from_layout(const word addr) {
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

void bus_device::groundpin(const int pin) {
	pinout.pins[pin] = -2;
	processlayout = true;
}

void bus_device::vccpin(const int pin) {
	pinout.pins[pin] = -1;
	processlayout = true;
}

void bus_device::swappins(const int pin1, const int pin2) {
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
	return devicedescriptor;
}

void device::dma(byte *data, bool is_output, bool started) {
	// dma transfer.
}


void bus_device::write(const int addr, const int addr_from_base, const byte data) {
	
}

byte bus_device::read(int const addr, const int addr_from_base, const bool onlyread) {
	return 0x00;
}

void bus_device::_attach_to_bus(bus * attachedbus) {
	devicebus = attachedbus;
}

device::device() {
	devicedescriptor = (char *)malloc(MAX_DESCRIPTOR_LENGTH);	// reserve 128 bytes.
	strncpy(get_device_descriptor(), "Denver Base Device", MAX_DESCRIPTOR_LENGTH);
	ticksdone = 0;
	tickstodo = 0;
}

device::~device() {
	free(devicedescriptor);	// be done with it.
}

int device::rundevice(const int ticks) {
	return ticks;	// dummy device return same amount of ticks as told to process.
}

void device::reset() {

}