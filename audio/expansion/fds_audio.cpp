/*

	FDS audio implementation.

*/

#include "fds_audio.h"
#include <iostream>

#pragma warning(disable : 4996)

fdsaudio::fdsaudio() {
	strncpy(get_device_descriptor(), "Denver FDS Audiochip", MAX_DESCRIPTOR_LENGTH);
	for (int i = 0; i < 64; i++) waveram[i] = 0;
	devicestart = 0x4040;
	deviceend = 0x409F;
	devicemask = 0x40FF;
	set_debug_data();
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
		volume_resetTimer();
		if (volenv_gainmode) volenv_gain_gain = volenv_gain_speed;
		return;
		break;		
	case 0x4082: // freq low
		freq = (freq & 0x0F00) | data;
		return;
		break;
	case 0x4083: // freq high + sweep / envelope settings.
		freq = (freq & 0x00FF) | ((data & 0x0F) << 8);
		disable_volsweep = (data & 0x40) > 0;
		env4xboost_stopmod = (data & 0x80) > 0;
		return;
		break;
	case 0x4084: // mod settings.
		modenv_gain_speed = data & 0x3F;
		modenv_gainmode = (data & 0x80) > 0;
		modenv_direction = (data & 0x40) >> 6;
		mod_resetTimer();
		if (modenv_gainmode) modenv_gain_gain = modenv_gain_speed;
		return;
		break;
	case 0x4085: // mod counter.
		mod_updateCounter(data & 0x7F);
		return;
		break;
	case 0x4086: // mod freq low.
		mod_freq = (mod_freq & 0x0F00) | data;
		return;
		break;
	case 0x4087: // mod freq high + settings.
		mod_freq = (mod_freq & 0x00FF) | ((data & 0x0F) << 8);
		carry_bit11_mod = (data & 0x40) > 0;
		halt_mod_counter = (data & 0x80) > 0;
		if (halt_mod_counter) {
			mod_overflowcounter = 0;
			// ToDo: Undo this hack and find the issue that keeps mod_output stuck when halt_mod_counter is active.
			mod_output = 0;
		}
		return;
		break;
	case 0x4088: // mod table set.
		mod_writeTable(data);
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
		return 0x40 | volenv_gain_gain;
		break;
	case 0x4091: // wave accumulator
		return (wave_accumulator >> 12) & 0xFF;
		break;
	case 0x4092: // mod gain
		return 0x40 | modenv_gain_gain;
		break;
	case 0x4093: // modtable addr accum
		return (mod_accumulator >> 5) & 0x7F;
		break;
	case 0x4094: // mod count * gain result.
		break;

	}
	return 0x40;
}

void	fdsaudio::mod_resetTimer() {
	mod_timer = 8 * (modenv_gain_speed + 1) * envelope_speed;
}

void	fdsaudio::volume_resetTimer() {
	vol_timer = 8 * (volenv_gain_speed + 1) * envelope_speed;
}

bool	fdsaudio::volume_tickEnvelope() {
	if (!volenv_gainmode && envelope_speed > 0) {
		vol_timer--;
		if (vol_timer <= 0) {
			volume_resetTimer();
			if (volenv_direction && volenv_gain_gain < 32) {
				volenv_gain_gain++;
			}
			else if (!volenv_direction && volenv_gain_gain > 0) {
				volenv_gain_gain--;
			}
		return true;
		}
	}
	return false;
}

void	fdsaudio::mod_updateCounter(int8_t data) {
	modulatorcount = data;
	if (modulatorcount >= 64) {
		modulatorcount -= 128;
	}
	else if (modulatorcount < -64) {
		modulatorcount += 128;
	}
}

bool	fdsaudio::mod_tickEnvelope() {
	if (!modenv_gainmode && envelope_speed > 0) {
		mod_timer--;
		if (mod_timer <= 0) {
			mod_resetTimer();
			if (modenv_direction && modenv_gain_gain < 32) {
				modenv_gain_gain++;
			}
			else if (!modenv_direction && modenv_gain_gain > 0) {
				modenv_gain_gain--;
			}
			return true;
		}
	}
	return false;
}

void	fdsaudio::mod_updateOutput(uint16_t ffreq) {
	// compute.
	int32_t	temp = modulatorcount * modenv_gain_gain;
	int32_t remainder = temp & 0x0F;
	temp >>= 4;
	if (remainder > 0 && (temp & 0x80) == 0) {
		temp += modulatorcount < 0 ? -1 : 2;
	}

	// clamp.
	if (temp >= 192) {
		temp -= 256;
	}
	else if (temp < -64) {
		temp += 256;
	}

	temp = ffreq * temp;
	remainder = temp & 0x3F;
	temp >>= 6;
	if (remainder >= 32) temp++;
	mod_output = temp;
}

void	fdsaudio::mod_writeTable(byte data) {
	if (halt_mod_counter) {
		modtable[mod_tableposition & 0x3F] = data & 0x07;
		modtable[(mod_tableposition + 1) & 0x3F] = data & 0x07;
		mod_tableposition = (mod_tableposition + 2) & 0x03f;
	}
}

bool	fdsaudio::mod_tickModulator() {
	if (!halt_mod_counter && (mod_freq > 0)) {
		mod_overflowcounter += mod_freq;
		if (mod_overflowcounter < mod_freq) {
			int32_t offset = modLuts[modtable[mod_tableposition]];
			mod_updateCounter(offset == 0xFF ? 0 : modulatorcount + offset);
			mod_tableposition = (mod_tableposition + 1) & 0x3F;
			return true;
		}
	}
	return false;
}

void	fdsaudio::clock() {
	if (!env4xboost_stopmod && !disable_volsweep) {
		volume_tickEnvelope();
		if (mod_tickEnvelope()) {
			mod_updateOutput(freq);
		}
	}

	if (mod_tickModulator()) {
		mod_updateOutput(freq);
	}

	if (env4xboost_stopmod) {
		wavePosition = 0;
		updateOutput();
	}
	else {
		updateOutput();
		if (freq + mod_output > 0 && !wavetable_write_enable) {
			waveOverflowCounter += freq + mod_output;
			if (waveOverflowCounter < freq + mod_output) {
				wavePosition = (wavePosition + 1) & 0x3F;
			}
		}
	}
}

void	fdsaudio::updateOutput() {
	uint32_t level = std::min((int)volenv_gain_gain, 32) * waveVolTable[master_volume];
	byte	outputLevel = (waveram[wavePosition] * level) / 1152;

	// convert to float.
	float	sample = (float)outputLevel / 128.0f;
	sample_buffer.push_back(sample);
}

int		fdsaudio::rundevice(int ticks) {
	for (int i = 0; i < ticks; i++) clock();
	return ticks;
}

void	fdsaudio::set_debug_data() {
	debugger.add_debug_var("FDS Audio", -1, NULL, t_beginblock);
	debugger.add_debug_var("Volume Speed", 64, &volenv_gain_speed, t_byte);
	debugger.add_debug_var("Volume Gain", 64, &volenv_gain_gain, t_byte);
	debugger.add_debug_var("Volume Gain Mode", -1, &volenv_gainmode, t_bool);
	debugger.add_debug_var("Volume Up Direction", -1, &volenv_direction, t_bool);
	debugger.add_debug_var("Sample Frequency", -1, &freq, t_word);

	debugger.add_debug_var("Modulate Speed", 64, &modenv_gain_speed, t_byte);
	debugger.add_debug_var("Modulate Gain", 64, &modenv_gain_gain, t_byte);
	debugger.add_debug_var("Modulate Gain Mode", -1, &modenv_gainmode, t_bool);
	debugger.add_debug_var("Modulate Up Direction", -1, &modenv_direction, t_bool);
	debugger.add_debug_var("Modulate Frequency", -1, &mod_freq, t_word);

	debugger.add_debug_var("Modulator count", -1, &modulatorcount, t_byte);
	debugger.add_debug_var("Halt modulator", -1, &halt_mod_counter, t_bool);
	debugger.add_debug_var("Carry bit 11 modulator", -1, &carry_bit11_mod, t_bool);

	debugger.add_debug_var("Master volume", -1, &master_volume, t_byte);
	debugger.add_debug_var("Wavetable Write Enable", -1, &wavetable_write_enable, t_bool);
	debugger.add_debug_var("Envelope master speed", -1, &envelope_speed, t_byte);
	debugger.add_debug_var("Extra Volume Gain", -1, &volume_gain, t_byte);

	debugger.add_debug_var("Accumulator Wave", 4, &wave_accumulator, t_bytearray);
	debugger.add_debug_var("Accumulator Modulator", 4, &mod_accumulator, t_bytearray);
	debugger.add_debug_var("Timer Volume", 4, &vol_timer, t_uint32);
	debugger.add_debug_var("Timer Modulator", 4, &mod_timer, t_uint32);

	debugger.add_debug_var("Wave table position", 64, &wavePosition, t_byte);
	debugger.add_debug_var("Modulator table position", 64, &mod_tableposition, t_byte);
	debugger.add_debug_var("Modulator output", -1, &mod_output, t_int32);

	debugger.add_debug_var("FDS Audio", -1, NULL, t_endblock);

	debugger.add_debug_var("FDS Audio Memory", -1, NULL, t_beginblock);
	debugger.add_debug_var("Wavetable RAM", 64, &waveram, t_bytebigarray);
	debugger.add_debug_var("Modulate Table", 64, &modtable, t_bytebigarray);
	debugger.add_debug_var("FDS Audio Memory", -1, NULL, t_endblock);
}