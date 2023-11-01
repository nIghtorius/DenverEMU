/*

	VRC6 implementation.

*/


#include "vrc6.h"
#include <iostream>

#pragma warning(disable : 4996)

// pulse channel

void	vrc6_pulse::update_timers() {
	if (frequency_counter == 0) {
		word nfreq = frequency;
		if (freq_16x) nfreq = frequency >> 4;
		if (freq_256x) nfreq = frequency >> 8;
		frequency_counter = nfreq;
		duty_pos++;
		if (duty_pos == 16) duty_pos = 0;
	} else frequency_counter--;
}

byte	vrc6_pulse::readsample() {
	if (ignore_duty) return volume;
	return vrc6_duty_cycle_osc[duty_cycle] & (1 << duty_pos) ? volume : 0;
}

// saw channel.
void	vrc6_saw::update_timers() {
	if (frequency_counter == 0) {
		word nfreq = frequency;
		if (freq_16x) nfreq = frequency >> 4;
		if (freq_256x) nfreq = frequency >> 8;
		frequency_counter = nfreq;
		step++;
		if (!(step & 1)) {
			accumulated += accumulator_rate;
		}
		if (step == 14) {
			step = 0;
			accumulated = 0;
		}
	}
	else frequency_counter--;
}

byte	vrc6_saw::readsample() {
	return (accumulated >> 3) & 0x1F;
}

// vrc6 chip implementation.

vrc6audio::vrc6audio() {
	strncpy(get_device_descriptor(), "Denver VRC6 Audio Chip", MAX_DESCRIPTOR_LENGTH);
	sample_buffer.clear();
	devicestart = 0x8000;
	deviceend = 0xBFFF;
	devicemask = 0xFFFF;
}

void	vrc6audio::write(int addr, int addr_from_base, byte data) {
	word taddr = addr;
	if (vrc6_mapper_026) {
		word a0 = (addr & 1) << 1;
		word a1 = (addr & 2) >> 1;
		taddr = addr & ~3;
		taddr = taddr | a0 | a1;
	}

	if (taddr == 0x9003) {
		halt_all_osc = (data & 1) > 0;
		pulse[0].freq_16x = (data & 2) > 0;
		pulse[1].freq_16x = (data & 2) > 0;
		pulse[0].freq_256x = (data & 4) > 0;
		pulse[1].freq_256x = (data & 4) > 0;
		saw.freq_16x = (data & 2) > 0;
		saw.freq_256x = (data & 4) > 0;
		return;
	}

	if ((taddr == 0x9000) || (taddr == 0xA000)) {
		byte device = (taddr & (0x3000)) >> 12;
		device--;
		pulse[device].volume = data & 0x0F;
		pulse[device].duty_cycle = (data >> 4) & 3;
		pulse[device].ignore_duty = (data & 0x80) > 0;
		return;
	}

	if (taddr == 0xB000) {
		saw.accumulator_rate = data & 0x3F;
		return;
	}

	if ((taddr == 0x9001) || (taddr == 0xA001)) {
		byte device = (taddr & (0x3000)) >> 12;
		device--;
		pulse[device].frequency &= ~0xFF;
		pulse[device].frequency |= data;
		return;
	}

	if ((taddr == 0x9002) || (taddr == 0xA002)) {
		byte device = (taddr & (0x3000)) >> 12;
		device--;
		pulse[device].frequency &= ~0xFF00;
		pulse[device].frequency |= (data & 0x0F) << 8;
		pulse[device].enable = (data & 0x80) > 0;
		return;
	}

	if (taddr == 0xB001) {
		saw.frequency &= ~0xFF;
		saw.frequency |= data;
		return;
	}

	if (taddr == 0xB002) {
		saw.frequency &= ~0xFF00;
		saw.frequency |= (data & 0x0F) << 8;
		saw.enable = (data & 0x80) > 0;
		return;
	}
}

void	vrc6audio::reset() {
	pulse[0].duty_pos = 0;
	pulse[1].duty_pos = 0;
	pulse[0].enable = false;
	pulse[1].enable = false;
	saw.enable = false;
	saw.step = 0;
}

float	vrc6audio::mux(byte p1, byte p2, byte sw) {
	float fp1, fp2, fsw;
	fp1 = (1.0f / 24) * (float)p1;
	fp2 = (1.0f / 24) * (float)p2;
	fsw = (1.0f / 48) * (float)sw;
	return (fp1 + fp2 + fsw) / 1.5f;
}

int		vrc6audio::rundevice(int ticks) {
	if (!ticks) return 0;
	for (int c = 0; c < ticks; c++) {
		if (!halt_all_osc) {
			for (int i = 0; i < 2; i++) pulse[i].update_timers();
			saw.update_timers();
		}
		// get samples.
		byte p1 = pulse[0].enable ? pulse[0].readsample() : 0;
		byte p2 = pulse[1].enable ? pulse[1].readsample() : 0;
		byte sw = saw.enable ? saw.readsample() : 0;
	
		sample_buffer.push_back(mux(p1, p2, sw));
	}
	return ticks;
}