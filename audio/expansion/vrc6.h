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
	bool		enable;
	bool		freq_16x;
	bool		freq_256x;	// 256x overrides 16x
	byte		volume;
	byte		duty_cycle;
	byte		duty_pos;
	bool		ignore_duty;
	word		frequency;
	word		frequency_counter;
	void		update_timers();
	byte		readsample();
};

struct vrc6_saw {
	bool		enable;
	bool		freq_16x;
	bool		freq_256x;	// 256x overrides 16x
	byte		accumulator_rate;
	byte		accumulated;
	word		frequency;
	word		frequency_counter;
	byte		step;
	void		update_timers();
	byte		readsample();
};

class vrc6audio : public audio_device {
private:
	bool		halt_all_osc;
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