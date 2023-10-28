// joypad.cpp

#include "joypad.h"
#include <iostream>

#pragma warning(disable : 4996)

joypad::joypad() {
	set_default_configs();
	for (int i = 0; i < MAX_CONTROLLERS; i++) {
		controllers[i] = { false, false, false, false, false, false, false, false };
	}
}

joypad::~joypad() {
}

void	joypad::set_default_configs() {
	cfg[0] = { 0x50, 0x52, 0x4f, 0x51, 0x1d, 0x1b, 0x04, 0x16 };
	cfg[1] = { 0x0d, 0x0c, 0x0f, 0x0e, 0x14, 0x1a, 0x08, 0x15 };
}

void	joypad::process_kb_event(SDL_Event *event) {
	bool buttonstate = (event->type == SDL_KEYDOWN);
	int  keycode = event->key.keysym.scancode;

	for (int i = 0; i < MAX_CONTROLLERS; i++) {
		if (cfg[i].left == keycode)
			controllers[i].states[1] = buttonstate;
		if (cfg[i].right == keycode)
			controllers[i].states[0] = buttonstate;
		if (cfg[i].up == keycode)
			controllers[i].states[3] = buttonstate;
		if (cfg[i].down == keycode)
			controllers[i].states[2] = buttonstate;
		if (cfg[i].a == keycode)
			controllers[i].states[7] = buttonstate;
		if (cfg[i].b == keycode)
			controllers[i].states[6] = buttonstate;
		if (cfg[i].select == keycode)
			controllers[i].states[5] = buttonstate;
		if (cfg[i].start == keycode)
			controllers[i].states[4] = buttonstate;
	}
}

void	joypad::strobe(int controller_id) {
	for (int i = 0; i < MAX_CONTROLLERS; i++) readouts[i] = 0x00;
}

bool	joypad::pulse_read_out(int controller_id) {
	if (readouts[controller_id] > 7) return 0;
	return controllers[controller_id].states[7-readouts[controller_id]++];
}

nes_2a03_joyports::nes_2a03_joyports() {
	strncpy(get_device_descriptor(), "Denver Controller 4016-4017", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;
}

nes_2a03_joyports::nes_2a03_joyports(joypad * ctrl) {
	strncpy(get_device_descriptor(), "Denver Controller 4016-4017", MAX_DESCRIPTOR_LENGTH);
	devicestart = 0x4000;
	deviceend = 0x401F;
	devicemask = 0x401F;
	controller = ctrl;
}

nes_2a03_joyports::~nes_2a03_joyports() {

}

void	nes_2a03_joyports::write(int addr, int addr_from_base, byte data)
{
	if (addr_from_base == CTR_STROBE_PORT) {
		controller->strobe(0);
	}
}

byte	nes_2a03_joyports::read(int addr, int addr_from_base, bool onlyread)
{
	switch (addr_from_base) {
		case CTR_CTRL1_PORT:
		case CTR_CTRL2_PORT:
			int controller_id = addr_from_base - CTR_CTRL1_PORT;
			if (controller->pulse_read_out(controller_id)) return 1;
			break;
	}
	return 0;
}
