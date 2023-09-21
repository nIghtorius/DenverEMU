#include "stdafx.h"
#include "apu.h"

apu::apu() {
	strcpy_s(this->get_device_descriptor(), MAX_DESCRIPTOR_LENGTH, "NES APU Device");
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;

	// initialize pulse
	pulse[0].pulse2 = false;
	pulse[1].pulse2 = true;

	// clocking info.
	tick_rate = 3;	// make it same as cpu. tick_rate is a divider against tick_rate 1
}

apu::~apu() {

}

byte	apu::read(int addr, int addr_from_base) {
	return BUS_OPEN_BUS;
}

void	apu::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base < 0x08) {
		// pulse channels.
		int pulse_sel = (addr_from_base & PULSE2) > 0;
		byte cmd = addr_from_base & 0x03;
		switch (cmd) {
		case PULSE_DUTY_CYCLE_LCH_VOLENV:
			pulse[pulse_sel].duty_cycle = (data & 0xC0) >> 6;
			pulse[pulse_sel].envelope_loop = (data & 0x20) > 0;
			pulse[pulse_sel].constant_volume = (data & 0x10) > 0;
			pulse[pulse_sel].volume_envelope = data & 0x0F;
			pulse[pulse_sel].envelope_reload = true;
			break;
		case PULSE_SWEEP:
			pulse[pulse_sel].sweep_enable = (data & 0x80) > 0;
			pulse[pulse_sel].sweep_divider = (data & 0x70) >> 4;
			pulse[pulse_sel].sweep_negate = (data & 0x08) > 0;
			pulse[pulse_sel].sweep_shift = (data & 0x07);
			pulse[pulse_sel].sweep_reload = true;
			break;
		case PULSE_TIMER:
			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x0700) | data;
			break;
		case PULSE_LCL_TIMER:
			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x00FF) | ((data & 0x07) << 8);
			pulse[pulse_sel].length_counter = (data & 0xF8) >> 3;
			break;
		}
	}
}

int		apu::rundevice(int ticks) {
	return ticks;
}

// generators.
void	pulse_generator::update_timers() {
	// counter(s)
	if (timer_counter == 0) {
		timer_counter = timer;
		duty_pos--;
		if (duty_pos == 0xFF) duty_pos = 7;
	} else timer_counter--;
}

void	pulse_generator::sweep() {
	if (!sweep_enable) return;
	if ((timer < 8) || (!sweep_negate && (timer >= 0x078B))) return;
	if (sweep_div_count > 0) {
		sweep_div_count--;
	}
	else {
		sweep_div_count = sweep_divider;
		timer = sweep_negate ? timer - ((timer >> sweep_shift) + (int)pulse2) : timer + (timer >> sweep_shift);
	}
	if (sweep_reload) {
		sweep_reload = false;
		sweep_div_count = sweep_divider;
	}
}

void	pulse_generator::envelopes() {
	if (envelope_reload) {
		envelope_count = volume_envelope;
		envelope_out = 0x0F;
		envelope_reload = false;
		return;
	}
	if (envelope_count > 0) {
		envelope_count--;
	}
	else {
		envelope_count = volume_envelope;
		envelope_out = (envelope_out > 0) ? envelope_out - 1 : ((envelope_loop ? 0x0F : envelope_out));
	}
}

void	pulse_generator::half_clock() {
	// sweeper.
	sweep();
	// length counter.
	if (!envelope_loop && (length_counter > 0)) length_counter--;
}

void	pulse_generator::quarter_clock() {
	envelopes();
}

byte	pulse_generator::readsample() {
	// muted conditions.
	if ((timer < 8) || (!sweep_negate && (timer >= 0x078B))) return 0;
	if (length_counter == 0) return 0;

	// constant volume or envelope out?
	byte	output_level = volume_envelope;
	if (!constant_volume) output_level = envelope_out;
	bool duty_out = (1 << (7 - duty_pos)) & duty_cycle_osc[duty_cycle];
	return duty_out ? output_level : 0;
}