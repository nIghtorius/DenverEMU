#include "clock.h"
#include <iostream>

clock::clock() {
}


clock::~clock() {
}

void clock::registerdevice(device *adevice) {
	this->devices.push_back(adevice);
	adevice->ticker = adevice->tick_rate;
}

void clock::step() {
	int cticks;
	int	maxticks = 0;

	// get device in dma mode.
	device *dmadevice = nullptr;
	for (auto device : this->devices) if (device->in_dma_mode) dmadevice = device;

	// DMA buffer.
	byte dmabyte;

	// execute one step of devices.. may be multiple clockticks to complete.
	for (auto device : this->devices) {
		device->ticker--;
		if (!device->ticker) {
			// check dma
			if (dmadevice == device) {
				device->dma(&dmabyte, false, device->dma_start);
				for (auto device : this->devices) if (device != dmadevice) device->dma(&dmabyte, true, device->dma_start);
				device->dma_start = false;
			}
			device->ticker = device->tick_rate;
			cticks = device->rundevice(1);	// always run one tick. problem is.. not all device do 1 tick so we always need to sync afterwards.
			device->ticksdone += cticks;
			if (cticks > maxticks) maxticks = cticks;
			device->tickstodo += cticks;	// 1/2 of todo.		(negative is good, it means we are done, 0 is best, means we are synced!)
		}
		else {
			// sleeper device.
			//device->tickstodo++;	// we sleep. therefor we needed to catch up (1 tick away)
			//device->ticksdone++;		// count those clocks as well!
		}
	}
	// one device has done something maximal. we compute this to all devices.
	bool	resync = true;
	while (resync) {
		resync = false;
		for (auto device : this->devices) {
			device->tickstodo -= maxticks;	// sync clock to all devices.
		}
		maxticks = 0;
		for (auto device : this->devices) {
			if (device->tickstodo <= -device->tick_rate) {
				// we are getting seriously desynced! resync
				cticks = device->rundevice(-device->tickstodo);
				device->ticksdone += cticks;
				if (cticks + device->tickstodo > maxticks) maxticks = cticks + device->tickstodo;
				device->tickstodo += cticks;
				resync = true;
			}
		}
	}
}

void clock::run() {
	// make it all run.
	while (1) {
		step();
	}
}

// fast clock
fastclock::fastclock() {
}

fastclock::~fastclock() {
}

void fastclock::setdevices(device *cpu, device *ppu, device *rom) {
	this->cpudevice = cpu;
	this->ppudevice = ppu;
	this->romdevice = rom;
}

void fastclock::step() {
	//  run cpu 1 step.
	byte dmabyte;
	if (cpudevice == NULL) return;
	if (cpudevice->in_dma_mode) {
		while (cpudevice->in_dma_mode) {
			cpudevice->dma(&dmabyte, false, false);
			if (ppudevice) ppudevice->dma(&dmabyte, true, cpudevice->dma_start);
			cpudevice->dma_start = false;
			int ticks = cpudevice->rundevice(1);
			if (romdevice) romdevice->rundevice(ticks / 3);			
		}
		if (ppudevice) ppudevice->rundevice(1536);
	}
	int actualcputicks = cpudevice->rundevice(cyclespersync);	
	if (romdevice) romdevice->rundevice(actualcputicks / 3);
	if (ppudevice) ppudevice->rundevice(actualcputicks);
}

void fastclock::run() {
	// make it all run.
	running = true;
	while (running) {
		//  run cpu 1 step.
		byte dmabyte;
		if (cpudevice == NULL) return;
		if (cpudevice->in_dma_mode) {
			while (cpudevice->in_dma_mode) {
				cpudevice->dma(&dmabyte, false, false);
				if (ppudevice) ppudevice->dma(&dmabyte, true, cpudevice->dma_start);
				cpudevice->dma_start = false;
				int ticks = cpudevice->rundevice(1);
				if (romdevice) romdevice->rundevice(ticks / 3);
			}
			ppudevice->rundevice(1536);
		}
		int actualcputicks = cpudevice->rundevice(cyclespersync);	
		if (romdevice) romdevice->rundevice(actualcputicks / 3);
		if (ppudevice) ppudevice->rundevice(actualcputicks);
	}
}

void fastclock::set_sync_cycle_in_ppucycles(int ppucycles) {
	cyclespersync = ppucycles;
}