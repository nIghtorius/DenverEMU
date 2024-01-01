/*

	Namco 163 Sound Chip
	(c) 2023/2024 P. Santing

*/

#pragma once

#include "../../bus/bus.h"
#include "../audio.h"
#include <vector>

// defines
#define		N163_RAM_LOW_FREQ			0x00
#define		N163_RAM_LOW_PHASE			0x01
#define		N163_RAM_MID_FREQ			0x02
#define		N163_RAM_MID_PHASE			0x03
#define		N163_RAM_HIGH_FREQ			0x04
#define		N163_RAM_WAVE_LENGTH		0x04
#define		N163_RAM_HIGH_PHASE			0x05
#define		N163_RAM_WAVE_ADDRESS		0x06
#define		N163_RAM_VOLUME				0x07

// magic tables.

// classes
struct n163_channel {
	byte *ram;
	byte ram_base = 0x40;	// set base address. (see defines)
	float output;
};

class namco163audio : public audio_device {
private:
	bool enable = false;
	byte address = 0x00;
	bool addr_incr = false;
	byte sound_ram[0x80];
	byte channels_active = 0x00;
	byte running_channel = 0x00;
	n163_channel	channels[8];
	byte update_tick = 0x00;
	void	update_channel(byte channel);
	float	output;
public:
	bool	enhanced_mixer = true;		// true better quality. acts not like original hardware.
	namco163audio();	
	void	reset();
	void	write(int addr, int addr_from_base, byte data);
	float	mux();
	int		rundevice(int ticks);
};