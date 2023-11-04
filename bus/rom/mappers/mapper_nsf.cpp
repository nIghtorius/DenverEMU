/*
	
	NSF implementation

*/

#include "mapper_nsf.h"
#include <iostream>

#pragma warning(disable : 4996)

nsfrom::nsfrom() {
	strncpy(get_device_descriptor(), "Denver NSF Player", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x0800;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	ram = (byte *)malloc(8192);		// 8K 0x6000-7FFF
	// copy in the defined uFirmware.
	memcpy(&ufirm[0], nsfufirm, sizeof(nsfufirm));
}

nsfrom::~nsfrom() {
	free(ram);
}

byte	nsfrom::read(int addr, int addr_from_base, bool onlyread) {
	// uFirmware.
	if ((addr >= 0x0800) && (addr <= 0x0800 + sizeof(nsfufirm))) return ufirm[addr - 0x0800];

	// custom vectors.
	if (addr == 0xFFFA) return state.nmi_vector & 0xFF; // low byte.
	if (addr == 0xFFFB) return (state.nmi_vector & 0xFF00) >> 8; // hi byte.
	if (addr == 0xFFFC) return state.res_vector & 0xFF; // low byte.
	if (addr == 0xFFFD) return (state.res_vector & 0xFF00) >> 8; // hi byte.
	if (addr == 0xFFFE) return state.irq_vector & 0xFF; // low byte.
	if (addr == 0xFFFF) return (state.irq_vector & 0xFF00) >> 8; // hi byte.

	// expanded RAM.
	if ((addr >= 0x6000) && (addr <= 0x7FFF)) return ram[addr - 0x6000];

	// banks. RAM is divided in 4K blocks.
	for (int i = 0; i < 8; i++) {
		int base = 0x8000 + (i * 0x1000);
		int end = 0x8FFF + (i * 0x1000);
		if ((addr >= base) && (addr <= end)) return prg[i][addr - base];
	}

	return 0;
}

void	nsfrom::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0x5FF8) && (addr <= 0x5FFF)) {
		byte	bank = addr - 0x5FF8;
		prg[bank] = &romdata[(data << 12) % romsize];
		return;
	}
}

int		nsfrom::rundevice(int ticks) {
	// for expansion audio.
	// check vrc6
	if (vrc6exp) vrc6exp->rundevice(ticks);
	if (sunexp) sunexp->rundevice(ticks);
	return ticks;
}

void	nsfrom::initialize(byte song) {
	// get the defaults from state.
	// first check "base_nsf" state. When the banks are all 0.

	bool	are_all_zero = true;
	for (int i = 0; i < 8; i++)
		if (state.banks[i] != 0) are_all_zero = false;

	if (are_all_zero) {
		// emulate old nsf.
		for (int i = 0; i < 8; i++) state.banks[i] = i;
	}

	// initialize banks.
	for (int i = 0; i < 8; i++) {
		prg[i] = &romdata[(state.banks[i] << 12) % romsize];
	}

	// perform a bus reset.
	devicebus->busreset();

	// initialize machine state.
	for (int i = 0; i < 0x0800; i++)
		devicebus->writememory(i, 0);
	
	for (int i = 0x6000; i < 0x8000; i++)
		devicebus->writememory(i, 0);

	for (int i = 0x4000; i < 0x4014; i++)
		devicebus->writememory(i, 0);

	devicebus->writememory(0x4015, 0);
	devicebus->writememory(0x4015, 0x0F);
	devicebus->writememory(0x4017, 0x40);

	// patch ufirmware.
	nsf_ufirmware_header		vectors;
	memcpy(&vectors, ufirm, sizeof(nsf_ufirmware_header));

	ufirm[vectors.trackselect-0x0800] = song;
	ufirm[vectors.init - 0x0800] = state.init & 0xFF;
	ufirm[vectors.init - 0x0800 + 1] = (state.init & 0xFF00) >> 8;
	ufirm[vectors.play - 0x0800] = state.play & 0xFF;
	ufirm[vectors.play - 0x0800 + 1] = (state.play & 0xFF00) >> 8;

	// update state.
	state.irq_vector = vectors.irq;
	state.nmi_vector = vectors.nmi;
	state.res_vector = vectors.reset;
}