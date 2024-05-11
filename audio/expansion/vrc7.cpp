/*

	Konami VRC7 Sound Chip
	(c) 2024 Multiple Authors.
	Denver Module written by P. Santing 

*/

#include "vrc7.h"
#include <cstdint>

#pragma warning(disable : 4996)

vrc7audio::vrc7audio() {
	strncpy(get_device_descriptor(), "Denver VRC7 Audio Chip", MAX_DESCRIPTOR_LENGTH);
	opl = OPLL_new(NES_CLOCK_SPEED_NTSC * 2, SAMPLE_RATE);
	if (opl != nullptr) {
		OPLL_set_rate(opl, SAMPLE_RATE);
		OPLL_reset(opl);
	}
	else {
		strncpy(get_device_descriptor(), "Denver VRC7 Audio - Failed to init", MAX_DESCRIPTOR_LENGTH);
	}
	devicestart = 0x9000;
	deviceend = 0x90FF;
	devicemask = 0x9FFF;
	// ToDo: Figure why 49716 sounds OK but not the SAMPLE_RATE (which should be the correct value)
	msamplehold = NES_CLOCK_SPEED_NTSC / 49716;
	msamplehold++;
}

vrc7audio::~vrc7audio() {
	if (opl != nullptr)
		OPLL_delete(opl);
}

void vrc7audio::reset() {
	if (opl != nullptr)
		OPLL_reset(opl);
}

void vrc7audio::write(int addr, int addr_from_base, byte data) {
	if (addr == 0x9010) {
		// Audio Register Select.
		regselect = data;
		return;
	}
	if (addr == 0x9030) {
		// Audio Register Write.
		if (opl!=nullptr)
			OPLL_writeReg(opl, regselect, data);
		return;
	}
}

int	vrc7audio::rundevice(int ticks) {
	// we have to render audio?
	// audio.h expects emulated samples per clock.
	if (!ticks) return 0;
	for (int c = 0; c < ticks; c++) {
		// get samples.
		if (samplehold == msamplehold) {
			if (opl != nullptr) {
				// if this goes right >_<
				sample = (float)((calc(opl)) << 2);
				sample /= 32768.0f;
				// normalize?
				if (sample < 0) {
					float updatenorm = -sample;
					if (updatenorm > normalize) normalize = updatenorm;
				}
			}
			samplehold = 0;
		}
		else {
			samplehold++;
		}
		sample_buffer.push_back(sample + normalize);
	}
	return ticks;
}
