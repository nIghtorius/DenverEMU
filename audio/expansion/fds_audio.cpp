/*

	FDS audio implementation.

*/

#include "fds_audio.h"

#pragma warning(disable : 4996)

fdsaudio::fdsaudio() {
	strncpy(get_device_descriptor(), "Denver FDS Audiochip", MAX_DESCRIPTOR_LENGTH);
	for (int i = 0; i < 64; i++) waveram[i] = 0;
	devicestart = 0x4040;
	deviceend = 0x409F;
	devicemask = 0x40FF;
}

void	fdsaudio::write(const int addr, const int addr_from_base, const byte data) {
	// waveram.
	if ((addr <= 0x407F) && wavetable_write_enable) {		
		waveram[addr & 0x3F] = data & 0x3F;	// 6 bit data, last 2 bits ignored.
	}
	switch (addr) {
	case 0x4080:	// volenv speed / gain / direction.
		volenv_gain_speed = data & 0x3F;
		volenv_gainmode = (data & 0x80) > 0;
		volenv_direction = (data & 0x40) >> 6;
		return;
		break;		
	case 0x4082: // freq low
		freq = (freq & 0xFF00) | data;
		return;
		break;
	case 0x4083: // freq high + sweep / envelope settings.
		freq = (freq & 0x00FF) | ((data & 0x0F) << 8);
		disable_volsweep = (data & 0x40) > 0;
		env4xboost = (data & 0x80) > 0;
		return;
		break;
	case 0x4084: // mod settings.
		modenv_gain_speed = data & 0x3F;
		modenv_gainmode = (data & 0x80) > 0;
		modenv_direction = (data & 0x40) >> 6;
		return;
		break;
	case 0x4085: // mod counter.
		modulatorcount = data & 0x7F;
		return;
		break;
	case 0x4086: // mod freq low.
		mod_freq = (mod_freq & 0xFF00) | data;
		return;
		break;
	case 0x4087: // mod freq high + settings.
		mod_freq = (mod_freq & 0x00FF) | ((data & 0x0F) << 8);
		carry_bit11_mod = (data & 0x40) > 0;
		halt_mod_counter = (data & 0x80) > 0;
		return;
		break;
	case 0x4088: // mod table set.
		if (!halt_mod_counter) return;
		mtable = data & 0x07;
		return;
		break;
	case 0x4089: // master volume.
		master_volume = (data & 0x03);
		wavetable_write_enable = (data & 0x80) > 0;
		return;
		break;
	case 0x408A: // env speed.
		envelope_speed = data;
		return;
		break;
	}
	return;
}

byte	fdsaudio::read(const int addr, const int addr_from_base, const bool onlyread) {
	// waveram
	if ((addr >= 0x4040) && (addr <= 0x407F)) {
		return 0x40 | waveram[addr & 0x3F];
	}
	switch (addr) {
	case 0x4090: // volume gain.
		return 0x40 | volenv_gain_speed;
		break;
	case 0x4091: // wave accumulator
		return (wave_accumulator >> 12) & 0xFF;
		break;
	case 0x4092: // mod gain
		return 0x40 | modenv_gain_speed;
		break;
	case 0x4093: // modtable addr accum
		return (mod_accumulator >> 5) & 0x7F;
		break;
	case 0x4094: // mod count * gain result.
		break;

	}
}