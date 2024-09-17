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
	set_debug_data();
}

nsfrom::~nsfrom() {
	free(ram);
}

byte	nsfrom::read(const int addr, const int addr_from_base, const bool onlyread) {
	// uFirmware.
	if ((addr >= 0x3000) && (addr <= 0x3000 + sizeof(nsfufirm))) return ufirm[addr - 0x3000];

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
		int base = 0x8000 | (i << 12);
		int end = base | 0x0FFF;
		if ((addr >= base) && (addr <= end)) return prg[i][addr - base];
	}

	return 0;
}

void	nsfrom::write(const int addr, const int addr_from_base, const byte data) {
	// enable NMI.
	if ((addr == 0x3000) && (data == 0xFF)) {
		nmi_enabled = true;
	}
	if ((addr >= 0x5FF8) && (addr <= 0x5FFF)) {
		byte	bank = addr - 0x5FF8;
		state.banks[bank] = data;
		prg[bank] = &romdata[(data << 12) % romsize];
		return;
	}
}

int		nsfrom::rundevice(const int ticks) {
	// for expansion audio.
	// check expansions
	if (vrc6exp) vrc6exp->rundevice(ticks);
	if (sunexp) sunexp->rundevice(ticks);
	if (namexp) namexp->rundevice(ticks);
	if (vrc7exp) vrc7exp->rundevice(ticks);
	if (mmc5exp) mmc5exp->rundevice(ticks);
	if (fdsexp) fdsexp->rundevice(ticks);

	tickcount += ticks;
	if (tickcount >= nmi_trig_cycles) {
		if (nmi_enabled) nmi_enable = true;
		tickcount -= nmi_trig_cycles;
	}
	return ticks;
}

void	nsfrom::set_rom_data(byte *data, const std::size_t size) {
	romdata = data;
	romsize = (int)size;
}

void	nsfrom::initialize(const byte song) {
	// get the defaults from state.
	// first check "base_nsf" state. When the banks are all 0.
	// first reset the tickcount to 0
	tickcount = 0;		// so we do not trip the NMI during NSFufirmware initialization.
	nmi_enable = false;	// make sure a triggered not handled NMI is suppressed.
	nmi_enabled = false; // NSF hardware do not trigger NMI's. write 0xff to 0x3000 to enable this. Done via uFirmware.

	// check if all zero and copy initial state to banks. (initial state is sbanks)
	bool	are_all_zero = true;
	for (int i = 0; i < 8; i++) {
		state.banks[i] = state.sbanks[i];
		if (state.banks[i] != 0) are_all_zero = false;
	}

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

	ufirm[vectors.trackselect - 0x3000] = song;
	ufirm[vectors.init - 0x3000] = state.init & 0xFF;
	ufirm[vectors.init - 0x3000 + 1] = (state.init & 0xFF00) >> 8;
	ufirm[vectors.play - 0x3000] = state.play & 0xFF;
	ufirm[vectors.play - 0x3000 + 1] = (state.play & 0xFF00) >> 8;

	// update state.
	state.irq_vector = vectors.irq;
	state.nmi_vector = vectors.nmi;
	state.res_vector = vectors.reset;

	timestarted = SDL_GetTicks64();
}

void	nsfrom::set_debug_data() {
	debugger.add_debug_var("NSF ROM", -1, NULL, t_beginblock);
	debugger.add_debug_var("Tickcount", -1, &tickcount, t_int);
	debugger.add_debug_var("Cycles to trigger NMI", -1, &nmi_trig_cycles, t_int);
	debugger.add_debug_var("NMI enabled", -1, &nmi_enabled, t_bool);
	debugger.add_debug_var("Expansion VRC6", -1, &vrc6exp, t_bool);
	debugger.add_debug_var("Expansion Sunsoft", -1, &sunexp, t_bool);
	debugger.add_debug_var("Expansion Namco", -1, &namexp, t_bool);
	debugger.add_debug_var("Expansion MMC5", -1, &mmc5exp, t_bool);
	debugger.add_debug_var("Expansion VRC7", -1, &vrc7exp, t_bool);
	debugger.add_debug_var("NSF ROM", -1, NULL, t_endblock);

	debugger.add_debug_var("State", -1, NULL, t_beginblock);
	debugger.add_debug_var("Starting Banks", 8, &state.sbanks[0], t_bytearray);
	debugger.add_debug_var("Current Banks", 8, &state.banks[0], t_bytearray);
	debugger.add_debug_var("Init address", -1, &state.init, t_addr);
	debugger.add_debug_var("Play address", -1, &state.play, t_addr);
	debugger.add_debug_var("Load address", -1, &state.load, t_addr);
	debugger.add_debug_var("Number of songs", -1, &state.numsongs, t_int);
	debugger.add_debug_var("Current song", -1, &state.currentsong, t_int);
	debugger.add_debug_var("IRQ Vector", -1, &state.irq_vector, t_addr);
	debugger.add_debug_var("NMI Vector", -1, &state.nmi_vector, t_addr);
	debugger.add_debug_var("RESET Vector", -1, &state.res_vector, t_addr);
	debugger.add_debug_var("State", -1, NULL, t_endblock);
}