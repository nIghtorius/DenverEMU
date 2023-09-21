/*

	NES APU (Audio Processing Unit)
	(c) 2023 P. Santing

*/

#pragma once

#include "..\bus\bus.h"

#define PULSE_DUTY_CYCLE_LCH_VOLENV			0x00
#define PULSE_SWEEP							0x01
#define PULSE_TIMER							0x02
#define PULSE_LCL_TIMER						0x03
#define PULSE1								0x00
#define PULSE2								0x04

// magic tables
static const unsigned __int8 duty_cycle_osc[] = {
	0x01, 0x03, 0x0F, 0xFC
};

// audio generators.
class pulse_generator {
public:
	byte	duty_cycle;
	byte	duty_pos;
	bool	envelope_loop;
	bool	constant_volume;
	byte	volume_envelope;
	word	timer;
	word	timer_counter;	// counts to 0 and resets to .timer
	byte	length_counter;
	bool	sweep_enable;
	byte	sweep_divider;
	byte	sweep_div_count;
	bool	sweep_negate;
	byte	sweep_shift;
	bool	sweep_reload;
	bool	envelope_reload;
	byte	envelope_count;
	byte	envelope_out;
	bool	pulse2;

	void	update_timers();
	void	half_clock();
	void	quarter_clock();

	void	sweep();
	void	envelopes();

	byte	readsample();
};

// classes
class apu : public bus_device {
private:
	pulse_generator pulse[2];		// pulse generators.

public:
	apu();
	~apu();
	byte	read(int addr, int addr_from_base);
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
};
