/*

	NES APU (Audio Processing Unit)
	(c) 2023 P. Santing


	ToDo:

		Move mixing code out of the APU and make a seperate module.
		This is required because of expansion audio such as VRC6/VRC7/Namco/MMC5 etc.

*/

#pragma once

#include "../bus/bus.h"
#include "audio.h"
#include <vector>
#include <fstream>

// pulse generators
#define PULSE_DUTY_CYCLE_LCH_VOLENV			0x00
#define PULSE_SWEEP							0x01
#define PULSE_TIMER							0x02
#define PULSE_LCL_TIMER						0x03
#define PULSE1								0x00
#define PULSE2								0x04

// triangle generator
#define TRIANGLE_LC_SETUP					0x08
#define TRIANGLE_TIMER						0x0A
#define TRIANGLE_LCL_TIMER					0x0B

// noise generator
#define NOISE_LCH_VOLENV					0x0C
#define NOISE_LOOP_PERIOD					0x0E
#define NOISE_LENGTH_COUNTER				0x0F

// DMC
#define DMC_IRQ_LOOP_FREQ					0x10
#define DMC_DLOAD							0x11
#define DMC_SAMPLE_ADDR						0x12
#define DMC_SAMPLE_LENGTH					0x13

// APU ports.
#define APU_STATUS_REGISTER					0x15
#define APU_FRAME_COUNTER					0x17

// steps
#define APU_CLK_QUARTER						0x01
#define APU_CLK_HALF						0x02
#define APU_FRMCNT_RESET					0x04

// magic tables
static const byte duty_cycle_osc[] = {
	0x01, 0x03, 0x0F, 0xFC
};

static const byte triangle_osc[] = {
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

static const word noise_periods[] = {
	0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
	0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
};

static const word step_four_seq[] = {
	3728, 0x01, 7096, 0x03, 11185, 0x01, 14914, 0x03, 14915, 0x04
};

static const word step_five_seq[] = {
	3728, 0x01, 7096, 0x03, 11185, 0x01, 14914, 0x00, 18640, 0x03, 18641, 0x04
};

static const byte apu_length_table[] = {
	10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 18, 32, 30
};

static const word dmc_table[] = {
	0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6, 
	0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054,	0x0048, 0x0036
};

// audio generators.
class pulse_generator {
public:
	// values.
	bool	enabled;

	byte	duty_cycle;
	byte	duty_pos;
	bool	envelope_loop;
	bool	constant_volume;
	byte	volume_envelope;
	word	timer;
	word	timer_counter;	// counts to 0 and resets to .timer
	word	length_counter;
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
	bool	enabled;

	bool	triangle_length_loop;
	word	length_counter;	
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

class noise_generator {
public:
	// values
	bool	enabled;

	bool	envelope_loop;
	bool	constant_volume;
	byte	volume_envelope;
	byte	envelope_out;
	bool	envelope_reload;
	byte	envelope_count;
	bool	noise_loop;
	byte	noise_period;
	word	length_counter;
	word	timer_counter;
	word	noise_shift_reg = 0x01;
	// functions
	void	update_timers();
	void	half_clock();
	void	quarter_clock();
	void	envelopes();
	byte	readsample();
};

class dmc_generator {
public:
	// values
	bool	enabled;

	bool	irq_enable;
	bool	irq_asserted = false;
	bool	dmc_loop;
	word	rate;
	byte	direct_out;
	word	sample_addr;
	word	sample_addr_counter;
	word	sample_length_load;
	word	sample_length;
	word	count;
	byte	sample_buffer;
	bool	sample_buffer_ready;
	bool	silent;
	byte	sample_shift_register;
	byte	bits_in_sample_remaining;

	bus					*mainbus;

	// functions
	byte	readsample();
	void	update_timers();
};

// classes
class apu : public audio_device {
private:
	float				pulse_muxtable[32];
	float				tnd_table[204];

	bool				frame_irq_asserted; // irq
	bool				five_step_mode;
	bool				inhibit_irq;

	void				half_clock();
	void				quarter_clock();

	int					framecycle = 0;
	int					frame_counter = 0;

	int					sample_buffer_counter = 0;

	float				mux(byte p1, byte p2, byte tri, byte noi, byte dmc);

public:
	apu();
	~apu();

	pulse_generator		pulse[2];		// pulse generators.
	triangle_generator	triangle;		// triangle generator.
	noise_generator		noise;			// noise generator.
	dmc_generator		dmc;			// delta modulation (sampling)

	byte	read(int addr, int addr_from_base);
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
	void	reset();
};
