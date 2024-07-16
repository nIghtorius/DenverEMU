#include "mapper_073.h"
#include <iostream>

/*

	Implementation of the VRC3 mapper.

*/

#pragma warning (disable : 4996)

vrc3rom::vrc3rom() {
	strncpy(get_device_descriptor(), "Denver VRC3 ROM", MAX_DESCRIPTOR_LENGTH);
	ram = (byte*)malloc(8192);
	devicestart = 0x6000;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
}

vrc3rom::~vrc3rom() {
	free(ram);
}

void vrc3rom::setbanks() {
	prg_8000 = &romdata[(state.prgbank << 14) % romsize];
	prg_c000 = &romdata[romsize - 16384];
}

byte vrc3rom::read(const int addr, const int addr_from_base, const bool onlyread) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) return ram[addr - 0x6000];	// read from ram.
	if ((addr >= 0x8000) && (addr <= 0xBFFF)) return prg_8000[addr - 0x8000];
	return prg_c000[addr - 0xC000];
}

void vrc3rom::write(const int addr, const int addr_from_base, const byte data) {
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) {
		ram[addr - 0x6000] = data;
		return;
	}
	if (addr >= 0xF000) {
		state.prgbank = data & 0x07;
		setbanks();
		return;
	}
	// IRQ latch.
	int action = addr >> 12;
	switch (action) {
	case 8:
		// IRQ latch 0
		state.latch = (state.latch & ~0x000F) | (data & 0x0F);
		break;
	case 9:
		// IRQ latch 1
		state.latch = (state.latch & ~0x00F0) | ((data & 0x0F) << 4);
		break;
	case 10:
		// IRQ latch 2
		state.latch = (state.latch & ~0x0F00) | ((data & 0x0F) << 8);
		break;
	case 11:
		// IRQ latch 3
		state.latch = (state.latch & ~0xF000) | ((data & 0x0F) << 12);
		break;
	case 12:
		// IRQ control
		irq_enable = false;
		state.irq_enable = (data & 0x02) > 0;
		state.irq_enable_ack = (data & 0x01) > 0;
		state.mode = (data >> 3) & 0x01;
		if (state.irq_enable) state.counter = state.latch;
		break;
	case 13:
		// IRQ ack.
		irq_enable = false;
		state.irq_enable = state.irq_enable_ack;
		break;
	}
}

void vrc3rom::set_rom_data(byte* data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
	setbanks();
}

int vrc3rom::rundevice(int ticks) {
	if (state.irq_enable) {
		for (int i = 0; i < ticks; i++) {
			state.counter++;
			if (state.mode == 0) {
				// 16 bit mode.
				if (state.counter == 0x0000) {
					irq_enable = true;
					state.counter = state.latch;
				}
			}
			else {
				// 8 bit mode.
				if ((state.counter & 0xFF) == 0x00) {
					irq_enable = true;
					state.counter = state.latch;
				}
			}
		}
	}
	return ticks;
}
