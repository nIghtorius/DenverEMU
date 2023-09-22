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

#define TRIANGLE_LC_SETUP					0x08
#define TRIANGLE_TIMER						0x0A
#define TRIANGLE_LCL_TIMER					0x0B

// magic tables
static const unsigned __int8 duty_cycle_osc[] = {
	0x01, 0x03, 0x0F, 0xFC
};

static const unsigned __int8 triangle_osc[] = {
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

// audio generators.
class pulse_generator {
public:
	// values.
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
	// functions
	void	update_timers();
	void	half_clock();
	void	quarter_clock();
	void	sweep();
	void	envelopes();
	byte	readsample();
};

class triangle_generator {
public:
	// values
	bool	triangle_loop;
	byte	length_counter;	
	byte	triangle_length;
	byte	triangle_length_counter;
	bool	triangle_counter_reload;
	word	timer;
	word	timer_counter;
	byte	sequencer;
	// functions
	void	update_timers();
	void	half_clock();
	void	quarter_clock();
	byte	readsample();
};

// classes
class apu : public bus_device {
private:
	pulse_generator		pulse[2];		// pulse generators.
	triangle_generator	triangle;		// triangle generator.

public:
	apu();
	~apu();
	byte	read(int addr, int addr_from_base);
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
};
