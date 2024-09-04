/*

	MMC5 sound implementation.

*/

#include "mmc5.h"
#include <iostream>
#include <string>

#pragma warning(disable : 4996)


mmc5audio::mmc5audio() {
	strncpy(get_device_descriptor(), "Denver MMC5 Audio", MAX_DESCRIPTOR_LENGTH);
	// setup the oscs to be MMC5 like.
	for (int i = 0; i < 2; i++) {
		pulse[i].sweep_enable = false;
		pulse[i].sweep_reload = false;		// disable sweep registers. they cannot be enabled on the MMC5 memory map.
	}
	devicestart = 0x5000;
	deviceend = 0x501F;
	devicemask = 0x501F;
	framecycle = 0;

	set_debug_data();
}

void	mmc5audio::quarter_clock() {
	pulse[0].half_clock();
	pulse[1].half_clock();
	pulse[0].quarter_clock();
	pulse[1].quarter_clock();
}

byte	mmc5audio::read(int addr, int addr_from_base, bool onlyread) {
	byte result = 0x00;
	switch (addr_from_base) {
	case DMC_IRQ_LOOP_FREQ:
		result = ((dmc.irq_asserted && dmc.irq_enable) ? 0x80 : 0x00) | (mmc5_dmc_read_mode ? 0x01 : 0x00);
		dmc.irq_asserted = false;
		break;
	case APU_STATUS_REGISTER:
		result = (pulse[0].length_counter ? 0x01 : 0) |
			(pulse[1].length_counter ? 0x02 : 0);
		return result;
		break;
	}
	return result;
}

void	mmc5audio::write(int addr, int addr_from_base, byte data) {
	if (addr_from_base < 0x08) {
		// pulse channels.
		int pulse_sel = (addr_from_base & PULSE2) >> 2;
		byte cmd = addr_from_base & 0x03;
		switch (cmd) {
		case PULSE_DUTY_CYCLE_LCH_VOLENV:
			pulse[pulse_sel].duty_cycle = (data & 0xC0) >> 6;
			pulse[pulse_sel].envelope_loop = (data & 0x20) > 0;
			pulse[pulse_sel].constant_volume = (data & 0x10) > 0;
			pulse[pulse_sel].volume_envelope = data & 0x0F;
			break;
		case PULSE_TIMER:
			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x0700) | data;
			break;
		case PULSE_LCL_TIMER:
			pulse[pulse_sel].timer = (pulse[pulse_sel].timer & 0x00FF) | ((data & 0x07) << 8);
			if (pulse[pulse_sel].enabled) pulse[pulse_sel].length_counter = apu_length_table[(data & 0xF8) >> 3];
			pulse[pulse_sel].duty_pos = 0; // restart sequencer.
			pulse[pulse_sel].envelope_reload = true; // restart envelope
			break;
		}
	}
	switch (addr_from_base) {
	case DMC_IRQ_LOOP_FREQ:
		mmc5_dmc_read_mode = (data & 0x01) > 0;
		dmc.irq_enable = (data & 0x80) > 0;
		break;
	case DMC_DLOAD:
		if (mmc5_dmc_read_mode) break;
		if (data == 0x00) {
			dmc.irq_asserted = true;
			break;
		}
		dmc.direct_out = data;
		break;
	case DMC_SAMPLE_ADDR:
		dmc.sample_addr = 0xC000 + (data << 6);
		dmc.sample_addr_counter = dmc.sample_addr;
		break;
	case DMC_SAMPLE_LENGTH:
		dmc.sample_length_load = (data << 4) | 1;
		break;
	case APU_STATUS_REGISTER:
		pulse[0].enabled = (data & 0x01) > 0;
		pulse[1].enabled = (data & 0x02) > 0;
		pulse[0].length_counter = pulse[0].enabled ? pulse[0].length_counter : 0;
		pulse[1].length_counter = pulse[1].enabled ? pulse[1].length_counter : 0;
		break;
	}
}

float	mmc5audio::mux(byte p1, byte p2, byte dmc) {
	return (((1.0f / 16.0f) * (float)p1) + ((1.0f / 16.0f) * (float)p2) + ((1.0f / 256.0f) * (float)dmc)) * 0.25f;
}

int		mmc5audio::rundevice(int ticks) {
	for (int i = 0; i < ticks; i++) {
		cTicks++;
		if (cTicks >= CPU_CYCLES_PER_FRAME_TICK) {
			cTicks -= CPU_CYCLES_PER_FRAME_TICK;
			quarter_clock();
		}
		if (framecycle % 2) pulse[0].update_timers();
		if (framecycle % 2) pulse[1].update_timers();
		dmc.update_timers();

		// get samples.
		byte p1 = pulse[0].enabled ? pulse[0].readsample() : 0;
		byte p2 = pulse[1].enabled ? pulse[1].readsample() : 0;
		byte dc = dmc.readsample();

		sample_buffer.push_back(mux(p1, p2, dc));

		framecycle++;
	}

	return ticks;
}

void	mmc5audio::reset() {
	for (int i = 0; i < 2; i++) {
		pulse[i].timer = 0;
		pulse[i].enabled = false;
		pulse[i].length_counter = 0;
		pulse[i].sweep_div_count = 0;
		pulse[i].envelope_count = 0;
	}
	dmc.enabled = false;
	dmc.direct_out = 0;
	mmc5_dmc_read_mode = false;
}

void	mmc5audio::set_debug_data() {
	debugger.add_debug_var("MMC5 AUDIO", -1, NULL, t_beginblock);

	debugger.add_debug_var("MMC5 DMC RO MODE", -1, &mmc5_dmc_read_mode, t_bool);
	debugger.add_debug_var("FRAMECYCLE CTR", -1, &framecycle, t_int);
	debugger.add_debug_var("CPU ticks CTR", -1, &cTicks, t_int);

	debugger.add_debug_var("MMC5 AUDIO", -1, NULL, t_endblock);

	for (int i = 1; i <= 2; i++) {
		std::string channelname = "Pulse channel #" + std::to_string(i);
		debugger.add_debug_var(channelname, -1, NULL, t_beginblock);
		debugger.add_debug_var("Enabled", -1, &pulse[i - 1].enabled, t_bool);
		debugger.add_debug_var("Timer", -1, &pulse[i - 1].timer, t_word);
		debugger.add_debug_var("Length counter", -1, &pulse[i - 1].length_counter, t_word);
		debugger.add_debug_var("Duty cycle", -1, &pulse[i - 1].duty_cycle, t_byte);
		debugger.add_debug_var("Duty pos", -1, &pulse[i - 1].duty_pos, t_byte);
		debugger.add_debug_var("Envelope count", -1, &pulse[i - 1].envelope_count, t_byte);
		debugger.add_debug_var("VolEnv", 15, &pulse[i - 1].volume_envelope, t_byte);
		debugger.add_debug_var("Constant volume", -1, &pulse[i - 1].constant_volume, t_bool);
		debugger.add_debug_var("EnvLoop", -1, &pulse[i - 1].envelope_loop, t_bool);
		debugger.add_debug_var("Envelope out", 15, &pulse[i - 1].envelope_out, t_byte);
		debugger.add_debug_var(channelname, -1, NULL, t_endblock);
	}

}
