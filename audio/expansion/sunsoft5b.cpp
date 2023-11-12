/*

	Implementation of sunsoft audio.

*/

#include "sunsoft5b.h"

#pragma warning(disable : 4996)

sunsoftaudio::sunsoftaudio() {
	strncpy(get_device_descriptor(), "Denver Sunsoft Audio", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0xC000;
	deviceend = 0xEFFF;
	devicemask = 0xFFFF;
}

void ym_channel::update_timers() {
	clock++;
	if (clock == 16) {
		period_counter++;
		if (period_counter >= period) {
			period_counter = 0;
			pulse = !pulse;
		}
		clock = 0;
	}
}

byte ym_channel::readsample() {
	return tone_disable ? 0 : pulse ? volume : 0;
}

void	sunsoftaudio::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0xC000) && (addr <= 0xDFFF)) {
		disable_e000 = (data & 0xF0) > 0;
		prgreg = data & 0x0F;
		return;
	}
	if (addr >= 0xE000) {
		if (disable_e000) return;

		switch (prgreg) {
		case YM_CHANNEL_A_PERIOD_LO:
			channels[0].period = (channels[0].period & 0xFF00) | data;
			break;
		case YM_CHANNEL_A_PERIOD_HI:
			channels[0].period = (channels[0].period & 0xFF) | ((data&0x0F) << 8);
			break;
		case YM_CHANNEL_B_PERIOD_LO:
			channels[1].period = (channels[1].period & 0xFF00) | data;
			break;
		case YM_CHANNEL_B_PERIOD_HI:
			channels[1].period = (channels[1].period & 0xFF) | ((data & 0x0F) << 8);
			break;
		case YM_CHANNEL_C_PERIOD_LO:
			channels[2].period = (channels[2].period & 0xFF00) | data;
			break;
		case YM_CHANNEL_C_PERIOD_HI:
			channels[2].period = (channels[2].period & 0xFF) | ((data & 0x0F) << 8);
			break;
		case YM_CHANNEL_NOISE_PERIOD:
			for (int i = 0; i < 3; i++)
				channels[i].noise_period = data & 0x1F;
			break;
		case YM_CHANNEL_DISABLE:
			channels[0].tone_disable = (data & 0x01) > 0;
			channels[1].tone_disable = (data & 0x02) > 0;
			channels[2].tone_disable = (data & 0x04) > 0;
			channels[0].noise_disable = (data & 0x08) > 0;
			channels[1].noise_disable = (data & 0x10) > 0;
			channels[2].noise_disable = (data & 0x20) > 0;
			break;
		case YM_CHANNEL_A_VOLENV:
		case YM_CHANNEL_B_VOLENV:
		case YM_CHANNEL_C_VOLENV:
			channels[prgreg - YM_CHANNEL_A_VOLENV].volume = (data & 0x0F);
			channels[prgreg - YM_CHANNEL_A_VOLENV].envelope_enable = (data & 0x10) > 0;
			break;
		case YM_CHANNEL_ENV_PERIOD_LO:
			for (int i = 0; i < 3; i++)
				channels[i].envelope_period = (channels[i].envelope_period & 0xFF00) | data;
			break;
		case YM_CHANNEL_ENV_PERIOD_HI:
			for (int i = 0; i < 3; i++)
				channels[i].envelope_period = (channels[i].envelope_period & 0xFF) | (data << 8);
			break;
		case YM_CHANNEL_ENV_MODE:
			for (int i = 0; i < 3; i++)
				channels[i].envelope_mode = data & 0x0F;
			break;
		}
	}
}

float	sunsoftaudio::mux(byte p1, byte p2, byte p3) {
	float fp1, fp2, fp3;
	fp1 = (1.0f / 32) * (float)p1;
	fp2 = (1.0f / 32) * (float)p2;
	fp3 = (1.0f / 32) * (float)p3;
	return (fp1 + fp2 + fp3) / 2.0f;
}

int		sunsoftaudio::rundevice(int ticks) {
	if (!ticks) return 0;
	for (int c = 0; c < ticks; c++) {
		for (int i = 0; i < 3; i++)
			channels[i].update_timers();

		// get samples.
		byte p1 = channels[0].readsample();
		byte p2 = channels[1].readsample();
		byte p3 = channels[2].readsample();

		// mix.
		sample_buffer.push_back(mux(p1, p2, p3));
	}
	return ticks;
}

void	sunsoftaudio::reset() {
	for (int i = 0; i < 3; i++) {
		channels[i].period_counter = 1;
		channels[i].tone_disable = false;
		channels[i].noise_disable = false;
	}
}
