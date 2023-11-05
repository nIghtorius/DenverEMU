#include "apu.h"
#include <iostream>

#pragma warning(disable : 4996)

apu::apu() {
	strncpy(get_device_descriptor(), "Denver 2A03 APU Device", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;

	// initialize pulse
	pulse[0].pulse2 = false;
	pulse[1].pulse2 = true;

	// clocking info.
	tick_rate = 3;	// make it same as cpu. tick_rate is a divider against tick_rate 1

	float maxnum = 0.0f;

	// prepare audio mux table.
	for (int i = 0; i < 32; i++) {
		pulse_muxtable[i] = -1.175196850393701f + (95.52f / (8128.0f / (i + 100)));
	}

	for (int i = 0; i < 204; i++) {
		tnd_table[i] = -0.6752341268204134f + (163.67f / (24239.0f / (i + 100)));
	}
}

apu::~apu() {

}

float	apu::mux(byte p1, byte p2, byte tri, byte noi, byte dmc) {
	return pulse_muxtable[p1 + p2] + tnd_table[3 * tri + 2 * noi + dmc];
}

byte	apu::read(int addr, int addr_from_base, bool onlyread) {
	switch (addr_from_base) {
	case APU_STATUS_REGISTER:
		byte result = (dmc.irq_asserted ? 0x80 : 0) |
			(frame_irq_asserted ? 0x40 : 0) |
			(pulse[0].length_counter ? 0x01 : 0) |
			(pulse[1].length_counter ? 0x02 : 0) |
			(triangle.length_counter ? 0x04 : 0) |
			(noise.length_counter ? 0x08 : 0);
		frame_irq_asserted = false;
		return result;
		break;
	}
	return 0;
}

void	apu::write(int addr, int addr_from_base, byte data) {
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
			//pulse[pulse_sel].envelope_reload = true;
			break;
		case PULSE_SWEEP:
			pulse[pulse_sel].sweep_enable = ((data & 0x80) > 0) && ((data & 0x07) != 0);
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
			pulse[pulse_sel].length_counter = apu_length_table[(data & 0xF8) >> 3];
			pulse[pulse_sel].duty_pos = 0; // restart sequencer.
			pulse[pulse_sel].envelope_reload = true; // restart envelope
			break;
		}
	}
	switch (addr_from_base) {
	case TRIANGLE_LC_SETUP:
		triangle.triangle_length_loop = (data & 0x80) > 0;
		triangle.triangle_length = data & 0x7F;
		break;
	case TRIANGLE_TIMER:
		triangle.timer = (triangle.timer & 0x0700) | data;
		break;
	case TRIANGLE_LCL_TIMER:
		triangle.timer = (triangle.timer & 0x00FF) | ((data & 0x07) << 8);
		triangle.length_counter = apu_length_table[(data & 0xF8) >> 3];
		triangle.triangle_counter_reload = true;
		break;
	case NOISE_LCH_VOLENV:
		noise.envelope_loop = (data & 0x20) > 0;
		noise.constant_volume = (data & 0x10) > 0;
		noise.volume_envelope = (data & 0x0F);
		break;
	case NOISE_LOOP_PERIOD:
		noise.noise_loop = (data & 0x80) > 0;
		noise.noise_period = (data & 0x0F);
		break;
	case NOISE_LENGTH_COUNTER:
		noise.length_counter = apu_length_table[(data & 0xF8) >> 3];
		noise.envelope_reload = true;
		break;
	case DMC_IRQ_LOOP_FREQ:
		dmc.irq_enable = (data & 0x80) > 0;
		dmc.dmc_loop = (data & 0x40) > 0;
		dmc.rate = dmc_table[(data & 0x0F)];
		if (!dmc.irq_enable) dmc.irq_asserted = false;
		dmc.count = 1;
		break;
	case DMC_DLOAD:
		dmc.direct_out = (data & 0x7F);
		break;
	case DMC_SAMPLE_ADDR:
		dmc.sample_addr = 0xC000 + (data << 6);
		dmc.sample_addr_counter = dmc.sample_addr;
		dmc.sample_buffer_ready = false;
		break;
	case DMC_SAMPLE_LENGTH:
		dmc.sample_length_load = (data << 4) | 1;
		if (dmc.sample_length == 0) dmc.sample_length = dmc.sample_length_load;
		break;
	case APU_STATUS_REGISTER:
		pulse[0].enabled =	(data & 0x01) > 0;
		pulse[1].enabled =	(data & 0x02) > 0;
		triangle.enabled =	(data & 0x04) > 0;
		noise.enabled = (data & 0x08) > 0;
		dmc.enabled = (data & 0x10) > 0;
		
		pulse[0].length_counter = pulse[0].enabled ? pulse[0].length_counter : 0;
		pulse[1].length_counter = pulse[1].enabled ? pulse[1].length_counter : 0;
		triangle.length_counter = triangle.enabled ? triangle.length_counter : 0;
		noise.length_counter = noise.enabled ? noise.length_counter : 0;

		if (!dmc.enabled) {
			dmc.sample_length = 0;
			dmc.count = 1;
		}
		else {
			if (dmc.sample_length == 0) dmc.sample_length = dmc.sample_length_load;
		}
		dmc.irq_asserted = false;
		break;
	case APU_FRAME_COUNTER:
		five_step_mode = (data & 0x80) > 0;
		inhibit_irq = (data & 0x40) > 0;
		if (inhibit_irq) frame_irq_asserted = false;
		if (five_step_mode) {
			half_clock();
			quarter_clock();
		}		
		break;
	}
}

void	apu::reset() {
	for (int i = 0; i < 2; i++) {
		pulse[i].timer = 0;
		pulse[i].enabled = false;
		pulse[i].length_counter = 0;
		pulse[i].sweep_div_count = 0;
		pulse[i].envelope_count = 0;
	}
	triangle.enabled = false;
	triangle.length_counter = 0;
	triangle.triangle_length_counter = 0;
	noise.enabled = false;
	noise.length_counter = 0;
	noise.volume_envelope = 0;
	dmc.enabled = false;
	dmc.direct_out = 0;
}

int		apu::rundevice(int ticks) {
	if (ticks == 0) return 0;
	while (ticks--) {
		// run the frame counter.
		word ctrigger = five_step_mode ? step_five_seq[frame_counter << 1] : step_four_seq[frame_counter << 1];
		byte cclock = five_step_mode ? (byte)step_five_seq[(frame_counter << 1) | 1] : (byte)step_four_seq[(frame_counter << 1) | 1];

		if (framecycle >= ctrigger*2) {
			frame_counter++;
			if (cclock & APU_CLK_QUARTER) {
				quarter_clock();
			}
			if (cclock & APU_CLK_HALF) {
				half_clock();
			}
			if (cclock & APU_FRMCNT_RESET) {
				framecycle = -1;
				frame_irq_asserted = (!inhibit_irq && !five_step_mode);
				frame_counter = 0;
				audioframes = 0;
				sample_buffer_counter++;
				if (sample_buffer_counter >= max_sample_buffer) {
					// tell audio_player when sample data is ready to go.
					audio_frame_ready = true;	// will be reset to false when picked up.
					sample_buffer_counter = 0;
				}
			}
		}

		if (audioframes >= (five_step_mode ? 37282 : 29830)) {
			audioframes = 0;
			sample_buffer_counter++;
			if (sample_buffer_counter >= max_sample_buffer) {
				audio_frame_ready = true;
				sample_buffer_counter = 0;
			}
		}

		// clock the osc's.
		if (framecycle % 2) pulse[0].update_timers();
		if (framecycle % 2) pulse[1].update_timers();
		triangle.update_timers();	// full speed clock.
		if (framecycle % 2) noise.update_timers();
		dmc.update_timers();		// full speed clock.

		// get sample(s)
		byte p1 = pulse[0].enabled ? pulse[0].readsample() : 0;
		byte p2 = pulse[1].enabled ? pulse[1].readsample() : 0;
		byte tr = triangle.enabled ? triangle.readsample() : 0;
		byte no = noise.enabled ? noise.readsample() : 0;
		ldm = dmc.enabled ? dmc.readsample() : ldm;

		sample_buffer.push_back(mux(p1, p2, tr, no, ldm));

		framecycle++;
		audioframes++;

		// check interrupts.
		this->irq_enable = frame_irq_asserted || dmc.irq_asserted;
	}
	return ticks;
}

void	apu::_attach_to_bus (bus * attachedbus) {
	dmc.mainbus = attachedbus;
	devicebus = attachedbus;
}

void	apu::half_clock() {
	pulse[0].half_clock();
	pulse[1].half_clock();
	triangle.half_clock();
	noise.half_clock();
}

void	apu::quarter_clock() {
	pulse[0].quarter_clock();
	pulse[1].quarter_clock();
	triangle.quarter_clock();
	noise.quarter_clock();
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
	if ((sweep_enable) && !((timer < 8) || (!sweep_negate && (timer >= 0x078B)))) 
	{
		if (sweep_div_count > 0) {
			sweep_div_count--;
		}
		else {
			sweep_div_count = sweep_divider;
			timer = sweep_negate ? timer - ((timer >> sweep_shift) + (int)pulse2) : timer + (timer >> sweep_shift);
		}
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
	}
	if (envelope_count > 0) {
		envelope_count--;
	}
	else {
		envelope_count = volume_envelope;
		envelope_out = (envelope_out > 0) ? envelope_out - 1 : ((envelope_loop ? 0x0F : 0));
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

void	triangle_generator::update_timers() {
	// counter(s)
	if (timer_counter == 0) {
		timer_counter = timer;
		// do we need to clock the sequencer? use modulus 32 to keep it in the 0-31 range without branching.
		if ((triangle_length_counter > 0) && (length_counter > 0) && (timer >= 3)) sequencer = (sequencer + 1) % 32;
	}
	else timer_counter--;
}

void	triangle_generator::half_clock() {
	// length counter.
	if (!triangle_length_loop && (length_counter > 0)) length_counter--;
}

void	triangle_generator::quarter_clock() {
	// another length counter??
	if (triangle_counter_reload) {
		triangle_length_counter = triangle_length;
	}
	else {
		triangle_length_counter = (triangle_length_counter > 0) ? triangle_length_counter - 1 : 0;
	}
	if (!triangle_length_loop) triangle_counter_reload = false;
}

byte	triangle_generator::readsample() {
	return triangle_osc[sequencer];
}

void	noise_generator::update_timers() {
	if (timer_counter == 0) {
		// reset timer period.
		timer_counter = noise_periods[noise_period];
		// compute feedback.
		word	feedback = noise_shift_reg & 0x01;
		if (noise_loop) {
			feedback ^= (noise_shift_reg & 0x40) >> 6;
		}
		else {
			feedback ^= (noise_shift_reg & 0x02) >> 1;
		}
		noise_shift_reg = (feedback << 14) | ((noise_shift_reg >> 1) & 0x7FFF);
	}
	else {
		timer_counter--;
	}
}

void	noise_generator::half_clock() {
	// length counter.
	if (!envelope_loop && (length_counter > 0)) length_counter--;
}

void	noise_generator::quarter_clock() {
	envelopes();
}

void	noise_generator::envelopes() {
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

byte	noise_generator::readsample() {
	if (!(noise_shift_reg & 0x01)) return 0;
	if (length_counter == 0) return 0;
	if (constant_volume) return volume_envelope;
	return envelope_out;
}

byte	dmc_generator::readsample() {
	return direct_out;
}

void	dmc_generator::update_timers() {
	count--;
	if (count < 2) {
		count = rate;
		if ((bits_in_sample_remaining > 0) & !silent) {
			if (sample_shift_register & 0x01) {
				if (direct_out <= 127) direct_out += 2;
			}
			else {
				if (direct_out >= 2) direct_out -= 2;
			}
			sample_shift_register >>= 1;
			bits_in_sample_remaining--;
		}
		if ((bits_in_sample_remaining == 0) || silent) {
			bits_in_sample_remaining = 8;
			if (!sample_buffer_ready) {
				silent = true;
			}
			else {
				silent = false;
				sample_shift_register = sample_buffer;
				sample_buffer_ready = false;
			}
		}
	}

	// get sample.
	if (!sample_buffer_ready) {
		if (sample_length > 0) {
			sample_buffer = mainbus->readmemory(sample_addr_counter);
			sample_buffer_ready = true;
			sample_addr_counter = (sample_addr_counter == 0xFFFF) ? 0x8000 : sample_addr_counter + 1;
			sample_length--;
			if (sample_length == 0) {
				if (dmc_loop) {
					sample_length = sample_length_load;
					sample_addr_counter = sample_addr;
				}
				else {
					if (irq_enable) irq_asserted = true;
					return;
				}
			}
		}
	}
}
