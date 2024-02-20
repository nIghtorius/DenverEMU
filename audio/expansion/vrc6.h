/*

	Konami VRC6 Sound chip
	(c) 2023 P. Santing

*/

#pragma once

#include "../../bus/bus.h"
#include "../audio.h"
#include <vector>

// magic tables.
static const byte vrc6_duty_cycle_osc[] = {
	1, 3, 7, 15, 31, 63, 127, 255
};

// classes

struct vrc6_pulse {
	bool		enable = false;
	bool		freq_16x = false;
	bool		freq_256x = false;	// 256x overrides 16x
	byte		volume = 0;
	byte		duty_cycle = 0;
	byte		duty_pos = 0;
	bool		ignore_duty = false;
	word		frequency = 0;
	word		frequency_counter = 0;
	void		update_timers();
	byte		readsample();
};

struct vrc6_saw {
	bool		enable = false;
	bool		freq_16x = false;
	bool		freq_256x = false;	// 256x overrides 16x
	byte		accumulator_rate = 0;
	byte		accumulated = 0;
	word		frequency = 0;
	word		frequency_counter = 0;
	byte		step = 0;
	void		update_timers();
	byte		readsample();
};

class vrc6audio : public audio_device {
private:
	bool		halt_all_osc = false;
	float		mux(byte p1, byte p2, byte sw);

public:
	vrc6audio();
	vrc6_pulse	pulse[2];
	vrc6_saw	saw;
	bool		vrc6_mapper_026 = false;
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
	void	reset();
};