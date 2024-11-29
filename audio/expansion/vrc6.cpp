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
	return (15-duty_pos) <= duty_cycle ? volume : 0;
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

float	vrc6_saw::readsample_hres() {
	byte p1 = (accumulated >> 3) & 0x1F;
	// precompute next step (p2)
	byte pacc = accumulated;
	if (step & 1) {
		pacc += accumulator_rate;
	}
	if (step == 13) {
		pacc = 0;
	}
	byte p2 = (pacc >> 3) & 0x1F;

	// exceptions (p1 == 0, return 0)
	// (p2 == 0) return p1
	if (p1 == 0) return 0.0f;
	if (p2 == 0) return (float)p1;

	int	neg_count = frequency - frequency_counter;
	if (neg_count > 0) {
		float to_p2 = (1.0f / frequency) * (float)neg_count;
		return ((1.0f - to_p2) * (float)p1) + (to_p2 * (float)p2);
	}
	else {
		return (float)p1;
	}
}

// vrc6 chip implementation.

vrc6audio::vrc6audio() {
	strncpy(get_device_descriptor(), "Denver VRC6 Audio Chip", MAX_DESCRIPTOR_LENGTH);
	sample_buffer.clear();
	devicestart = 0x8000;
	deviceend = 0xBFFF;
	devicemask = 0xFFFF;
	set_debug_data();
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
		pulse[device].duty_cycle = (data >> 4) & 7;
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
	for (int i = 0; i < 2; i++) {
		pulse[i].duty_pos = 0;
		pulse[i].enable = false;
		pulse[i].freq_16x = false;
		pulse[i].freq_256x = false;
		pulse[i].volume = 0;
		pulse[i].ignore_duty = false;
		pulse[i].frequency = 0;
		pulse[i].frequency_counter = 0;	
	}
	saw.enable = false;
	saw.freq_16x = false;
	saw.freq_256x = false;
	saw.step = 0;
	saw.accumulated = 0;
	saw.accumulator_rate = 0;
	saw.frequency = 0;
	saw.frequency_counter = 0;
	halt_all_osc = false;
}

float	vrc6audio::mux(byte p1, byte p2, byte sw) {
	float fp1, fp2, fsw;
	fp1 = (1.0f / 24) * (float)p1;
	fp2 = (1.0f / 24) * (float)p2;
	fsw = (1.0f / 48) * (float)sw;
	return (fp1 + fp2 + fsw) / 2.5f;
}

float	vrc6audio::hmux(byte p1, byte p2, float sw) {
	float fp1, fp2;
	fp1 = (1.0f / 24) * (float)p1;
	fp2 = (1.0f / 24) * (float)p2;
	return (fp1 + fp2 + (sw / 24.0f)) / 2.5f;
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
		if (!high_res) {
			sample_buffer.push_back(mux(p1, p2, sw));
		}
		else {
			float fsw = saw.enable ? saw.readsample_hres() : 0.0f;
			sample_buffer.push_back(mux(p1, p2, fsw));
		}
	}
	return ticks;
}

void	vrc6audio::set_debug_data() {
	debugger.add_debug_var("VRC6 Audio", -1, NULL, t_beginblock);
	debugger.add_debug_var("Halt all oscillators", -1, &halt_all_osc, t_bool);
	debugger.add_debug_var("Mapper 26 mode", -1, &vrc6_mapper_026, t_bool);
	debugger.add_debug_var("VRC6 Audio", -1, NULL, t_endblock);

	for (int i = 1; i <= 2; i++) {
		std::string channame = "VRC6 Pulse #" + std::to_string(i);
		debugger.add_debug_var(channame, -1, NULL, t_beginblock);
		debugger.add_debug_var("Enabled", -1, &pulse[i - 1].enable, t_bool);
		debugger.add_debug_var("Freq 16x mult", -1, &pulse[i - 1].freq_16x, t_bool);
		debugger.add_debug_var("Freq 256x mult", -1, &pulse[i - 1].freq_256x, t_bool);
		debugger.add_debug_var("Volume", 15, &pulse[i - 1].volume, t_byte);
		debugger.add_debug_var("Duty Cycle", -1, &pulse[i - 1].duty_cycle, t_byte);
		debugger.add_debug_var("Duty Pos", -1, &pulse[i - 1].duty_pos, t_byte);
		debugger.add_debug_var("Ignore Duty", -1, &pulse[i - 1].ignore_duty, t_bool);
		debugger.add_debug_var("Frequency", -1, &pulse[i - 1].frequency, t_word);
		debugger.add_debug_var("Frequency ctr", -1, &pulse[i - 1].frequency_counter, t_word);
		debugger.add_debug_var(channame, -1, NULL, t_endblock);
	}

	debugger.add_debug_var("VRC6 Sawtooth", -1, NULL, t_beginblock);
	debugger.add_debug_var("Enabled", -1, &saw.enable, t_bool);
	debugger.add_debug_var("Freq 16x mult", -1, &saw.freq_16x, t_bool);
	debugger.add_debug_var("Freq 256x mult", -1, &saw.freq_256x, t_bool);
	debugger.add_debug_var("Accumulator Rate", -1, &saw.accumulator_rate, t_byte);
	debugger.add_debug_var("Accumulated", -1, &saw.accumulated, t_byte);
	debugger.add_debug_var("Frequency", -1, &saw.frequency, t_word);
	debugger.add_debug_var("Frequency ctr", -1, &saw.frequency_counter, t_word);
	debugger.add_debug_var("Step", -1, &saw.step, t_byte);
	debugger.add_debug_var("VRC6 Sawtooth", -1, NULL, t_endblock);
}
