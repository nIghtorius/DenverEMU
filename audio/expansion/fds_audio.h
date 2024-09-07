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
	const int32_t modLuts[8] = { 0, 1, 2, 4, 0xFF, -4, -2, -1 };
	const uint32_t waveVolTable[4] = { 36, 24, 17, 14 };

	byte waveram[64] = {};
	byte modtable[64] = {};
	byte volenv_gain_speed = 0;
	byte volenv_gain_gain = 0;
	bool volenv_gainmode = false;
	byte volenv_direction = VOLENV_DECREASE;
	word freq = 0;
	bool env4xboost_stopmod = false;
	bool disable_volsweep = false;
	byte modenv_gain_speed = 0;
	byte modenv_gain_gain = 0;
	bool modenv_gainmode = false;
	byte modenv_direction = VOLENV_DECREASE;
	int8_t modulatorcount = 0x00;
	word mod_freq = 0;
	bool halt_mod_counter = false;
	bool carry_bit11_mod = false;
	byte master_volume = 0;
	bool wavetable_write_enable = false;
	byte envelope_speed = 0xE8;
	byte volume_gain = 0;
	uint32_t wave_accumulator = 0;
	uint32_t mod_accumulator = 0;
	uint32_t vol_timer = 0;
	uint32_t mod_timer = 0;
	int32_t mod_output = 0;
	uint16_t mod_overflowcounter = 0;
	byte	mod_tableposition = 0;
	byte	wavePosition = 0;
	uint16_t waveOverflowCounter = 0;
	
	void	clock();

	bool	volume_tickEnvelope();
	void	volume_resetTimer();

	bool	mod_tickEnvelope();
	void	mod_resetTimer();
	void	mod_updateCounter(int8_t data);
	void	mod_updateOutput(uint16_t freq);
	bool	mod_tickModulator();
	void	mod_writeTable(byte data);
	void	updateOutput();

public:
	fdsaudio();

	virtual void write(const int addr, const int addr_from_base, const byte data);
	virtual byte read(const int addr, const int addr_from_base, const bool onlyread = false);
	virtual int rundevice(int ticks);
	virtual void set_debug_data();
};