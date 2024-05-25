/*

	MMC5(a) Sound chip
	(c) 2024 P. Santing

*/

#pragma once

#include "../apu.h"	// GET APU functions. Most of it is the same.
#include "../audio.h"
#include <vector>

#define CPU_CYCLES_PER_FRAME_TICK	7445			// 240hz

class mmc5audio : public audio_device {
private:
	void				quarter_clock();
	int					cTicks = 0;
	int					framecycle = 0;

	float				mux(byte p1, byte p2, byte dmc);

public:
	pulse_generator pulse[2];		//mmc5 pulse generators, no sweeping units. borrowed from APU.
	dmc_generator	dmc;			//supports 8 bit samples, instead of 7 bit. borrowed from APU.

	bool			mmc5_dmc_read_mode = false;

	byte	read(int addr, int addr_from_base, bool onlyread = false);
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
	void	reset();
	virtual void	set_debug_data();

	mmc5audio();
};
