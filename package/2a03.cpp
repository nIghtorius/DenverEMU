/*

	implementation of package_2a03

*/

#include "2a03.h"

#pragma warning(disable : 4996)

package_2a03::package_2a03() {
	strncpy(get_device_descriptor(), "2A03 Package (CPU, APU, CTRL)", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;
}

void	package_2a03::set_joydefs(joypad * joydefs) {
	controllers.controller = joydefs;
}

byte	package_2a03::read(int addr, int addr_from_base, bool onlyread) {
	if ((addr_from_base == CTR_CTRL1_PORT) || (addr_from_base == CTR_CTRL2_PORT)) {
		return controllers.read(addr, addr_from_base, onlyread);
	}
	if (no_apu) return 0;
	return apu_2a03.read(addr, addr_from_base, onlyread);
}

void	package_2a03::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base == CPU_DMA) {
		cpu_2a03.write(addr, addr_from_base, data);
		in_dma_mode = cpu_2a03.in_dma_mode;
		return;
	}
	if (addr_from_base == CTR_STROBE_PORT) {
		controllers.write(addr, addr_from_base, data);
		return;
	}
	if (!no_apu) apu_2a03.write(addr, addr_from_base, data);
}

void	package_2a03::reset() {
	// reset linked devices.
	apu_2a03.reset();
	cpu_2a03.reset();
	controllers.reset();
}

int		package_2a03::rundevice(int ticks) {
	// leading device is the CPU.
	int cputicks = cpu_2a03.rundevice(ticks);
	if (!no_apu) {
		if (!nsf_mode) {
			apu_2a03.rundevice(cputicks / 3);	// cpu responds in ppu ticks, divide 3 to get cpu ticks.
		}
		else {
			apu_2a03.rundevice(cputicks / 3 / 3);	// oc cpu responds in ppu ticks, divide 3/3 to get cpu ticks.
		}
	}
	in_dma_mode = cpu_2a03.in_dma_mode; // take the dma status from the cpu object.
	// bugfix: 17-5-2024, Interrupts are not handled.
	irq_enable = apu_2a03.irq_enable || cpu_2a03.irq_enable || controllers.irq_enable;
	return cputicks;
}

void	package_2a03::dma(byte *data, bool is_output, bool started) {
	cpu_2a03.dma(data, is_output, started);	// forward dma mode to the cpu.
}

void	package_2a03::_attach_to_bus(bus * attachedbus) {
	// link the bus.
	cpu_2a03._attach_to_bus(attachedbus);
	if (!no_apu) apu_2a03._attach_to_bus(attachedbus);
	controllers._attach_to_bus(attachedbus);
}