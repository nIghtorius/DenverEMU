/*

	Famicom Disk System Audio chip.

*/

#pragma once

#include "../../bus/bus.h"
#include "../audio.h"
#include <vector>

#define VOLENV_DECREASE 0x00
#define VOLENV_INCREASE 0x01

class fdsaudio : public audio_device {
private:
	byte waveram[64];
	byte volenv_gain_speed = 0;
	bool volenv_gainmode = false;
	byte volenv_direction = VOLENV_DECREASE;
	word freq = 0;
	bool env4xboost = false;
	bool disable_volsweep = false;
	byte modenv_gain_speed = 0;
	bool modenv_gainmode = false;
	byte modenv_direction = VOLENV_DECREASE;
	byte modulatorcount = 0x00;
	word mod_freq = 0;
	bool halt_mod_counter = false;
	bool carry_bit11_mod = false;
	byte mtable = 0;
	byte master_volume = 0;
	bool wavetable_write_enable = false;
	byte envelope_speed = 0xE8;
	byte volume_gain = 0;
	uint32_t wave_accumulator = 0;
	uint32_t mod_accumulator = 0;


public:
	fdsaudio();

	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
};