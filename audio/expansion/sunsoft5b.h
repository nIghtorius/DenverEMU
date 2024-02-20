/*

	Sunsoft 5B? Sound chip
	(c) 2023 P. Santing

	No Full emulation. Only one game uses this. ( Gimmick )
	Uses only the pulse gens with no enveloping.

	Feel free to implement everything.

*/

#pragma once

#include "../../bus/bus.h"
#include "../audio.h"
#include <vector>

#define		YM_CHANNEL_A_PERIOD_LO			0x00
#define		YM_CHANNEL_A_PERIOD_HI			0x01
#define		YM_CHANNEL_B_PERIOD_LO			0x02
#define		YM_CHANNEL_B_PERIOD_HI			0x03
#define		YM_CHANNEL_C_PERIOD_LO			0x04
#define		YM_CHANNEL_C_PERIOD_HI			0x05
#define		YM_CHANNEL_NOISE_PERIOD			0x06
#define		YM_CHANNEL_DISABLE				0x07
#define		YM_CHANNEL_A_VOLENV				0x08
#define		YM_CHANNEL_B_VOLENV				0x09
#define		YM_CHANNEL_C_VOLENV				0x0A
#define		YM_CHANNEL_ENV_PERIOD_LO		0x0B
#define		YM_CHANNEL_ENV_PERIOD_HI		0x0C
#define		YM_CHANNEL_ENV_MODE				0x0D

// classes

struct ym_channel {
	word		period = 0;
	byte		clock = 0;
	word		period_counter = 0;
	byte		noise_period = 0;
	bool		noise_disable = false;
	bool		tone_disable = false;
	bool		envelope_enable = false;
	byte		volume = 0;
	word		envelope_period = 0;
	byte		envelope_mode = 0;
	bool		pulse = false;
	void		update_timers();
	byte		readsample();
};

class sunsoftaudio : public audio_device {
private:
	byte		prgreg = 0;
	bool		disable_e000 = false;
	float		mux(byte p1, byte p2, byte p3);
public:
	sunsoftaudio();
	ym_channel	channels[3];
	void	write(int addr, int addr_from_base, byte data);
	int		rundevice(int ticks);
	void	reset();
};

