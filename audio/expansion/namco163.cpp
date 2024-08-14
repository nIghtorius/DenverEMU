/*

	Namco 163 implementation.

*/


#include "namco163.h"
#include <iostream>

#pragma warning(disable : 4996)

namco163audio::namco163audio() {
	// set up channels.
	std::cout << "namco163audio::namco163audio (init)\n";

	for (int i = 0; i < 8; i++) {
		std::cout << "Setting up channel " << std::dec << 8 - i << " with base address 0x" << std::hex << ((int)0x78 - (i * 0x08)) << "\n";
		channels[i].ram_base = 0x78 - (i * 0x08);
	}
	// setup misc
	strncpy(get_device_descriptor(), "Denver Namco 163 Audio Chip", MAX_DESCRIPTOR_LENGTH);
	sample_buffer.clear();
	devicestart = 0x4800;
	deviceend = 0xFFFF;
	devicemask = 0xFFFF;
	set_debug_data();
}

void	namco163audio::reset() {
	memset(&sound_ram[0], 0, 0x80);
	channels_active = 0x00;
	running_channel = 0x00;
	address = 0x0000;
	addr_incr = false;
	update_tick = 0x00;
}

void	namco163audio::write(int addr, int addr_from_base, byte data) {
	if ((addr >= 0xE000) && (addr <= 0xE7FF)) {
		enable = (data & 0x40) == 0;	// bit6 = 0 (enable)
		return;
	}
	if ((addr >= 0xF800) && (addr <= 0xFFFF)) {
		address = data & 0x7F;
		addr_incr = (data & 0x80) > 0;
		return;
	}
	if ((addr >= 0x4800) && (addr <= 0x4FFF)) {
		//std::cout << "updating " << (int)address << " with " << (int)data << std::endl;
		sound_ram[address] = data;
		// check for channel setting.
		if (address == 0x7F) {
			channels_active = (data & 0x70) >> 4;
		}
		if (addr_incr) address++;
		address = address & 0x7F;
	}
}

int		namco163audio::rundevice(int ticks) {
	if (!ticks) return 0;
	for (int c = 0; c < ticks; c++) {
		update_tick++;
		if (update_tick == 15) {
			// update a channel.
			update_channel(running_channel);
			output = channels[running_channel].output;
			update_tick = 0;
			if (running_channel >= channels_active) {
				running_channel = 0;
			}
			else {
				running_channel++;
			}
		}
		// write to sample buffer.
		if (!enable) output = 0.0f;
		sample_buffer.push_back(mux());
	}
	return ticks;
}

float	namco163audio::mux() {
	if (!enhanced_mixer) return output;
	// enhanced mixer.
	float	final_mux = 0.0f;
	for (int i = 0; i < channels_active + 1; i++) {
		final_mux += channels[i].output;
	}
	//final_mux /= (1.0f+(channels_active >> 1));
	final_mux /= 2.0f;
	return final_mux;
}

void	namco163audio::update_channel(byte channel) {
	// decode ram slots.
	byte	baddr = channels[channel].ram_base;
	byte	sample_addr = sound_ram[baddr | N163_RAM_WAVE_ADDRESS];
	int		phase = sound_ram[baddr | N163_RAM_HIGH_PHASE] << 16 |
		sound_ram[baddr | N163_RAM_MID_PHASE] << 8 |
		sound_ram[baddr | N163_RAM_LOW_PHASE];
	int		freq = (sound_ram[baddr | N163_RAM_HIGH_FREQ] & 0x03) << 16 |
		sound_ram[baddr | N163_RAM_MID_FREQ] << 8 |
		sound_ram[baddr | N163_RAM_LOW_FREQ];
	int	length = 256 - (sound_ram[baddr | N163_RAM_WAVE_LENGTH] & 0xFC);
	byte	volume = sound_ram[baddr | N163_RAM_VOLUME] & 0x0F;

	// update phase.
	phase = (phase + freq) % ((int)length << 16);

	// get sample.
	int	samplepicker = ((phase >> 16) + sample_addr) & 0xFF;
	byte rawsample = (sound_ram[samplepicker >> 1] >> ((samplepicker & 1) << 2)) & 0x0F;
	channels[channel].output = (rawsample * volume) / 512.0f;

	// write phase back to sound_ram.
	sound_ram[baddr | N163_RAM_LOW_PHASE] = phase & 0xFF;
	sound_ram[baddr | N163_RAM_MID_PHASE] = (phase & 0xFF00) >> 8;
	sound_ram[baddr | N163_RAM_HIGH_PHASE] = (phase & 0xFF0000) >> 16;
}

void	namco163audio::set_debug_data() {
	debugger.add_debug_var("NAMCO 163", -1, NULL, t_beginblock);
	debugger.add_debug_var("Enabled", -1, &enable, t_bool);
	debugger.add_debug_var("Address pointer", -1, &address, t_byte);
	debugger.add_debug_var("Channels active", -1, &channels_active, t_byte);
	debugger.add_debug_var("Processing channel", -1, &running_channel, t_byte);
	debugger.add_debug_var("Update ticker", 15, &update_tick, t_byte);
	debugger.add_debug_var("Enhanced mixer", -1, &enhanced_mixer, t_bool);
	debugger.add_debug_var("NAMCO 163", -1, NULL, t_endblock);

	// buf
	char buf[16];

	// channels.
	for (int i = 0; i < 8; i++) {
		std::string desc = "Channel #" + std::to_string(i+1);
		debugger.add_debug_var(desc, -1, NULL, t_beginblock);

		debugger.add_debug_var("FREQ(24B)", -1, &sound_ram[channels[i].ram_base + N163_RAM_LOW_FREQ], t_24bit_n163);
		debugger.add_debug_var("PHASE(24B)", -1, &sound_ram[channels[i].ram_base + N163_RAM_LOW_PHASE], t_24bit_n163);

		debugger.add_debug_var("WAVELENGTH", -1, &sound_ram[channels[i].ram_base + N163_RAM_WAVE_LENGTH], t_byte);
		debugger.add_debug_var("ADDRESS(WT)", -1, &sound_ram[channels[i].ram_base + N163_RAM_WAVE_ADDRESS], t_byte);
		debugger.add_debug_var("VOLUME", 15, &sound_ram[channels[i].ram_base + N163_RAM_VOLUME], t_byte_lonibble);
		debugger.add_debug_var(desc, -1, NULL, t_endblock);
	}

	debugger.add_debug_var("Memory contents", -1, NULL, t_beginblock);
	debugger.add_debug_var("SOUNDRAM", 128, sound_ram, t_bytearray);
	debugger.add_debug_var("Memory contents", -1, NULL, t_endblock);
}
