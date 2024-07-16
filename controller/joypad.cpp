// joypad.cpp

#include "joypad.h"
#include <iostream>

#pragma warning(disable : 4996)

joypad::joypad() {
	set_default_configs();
	for (int i = 0; i < MAX_CONTROLLERS; i++) {
		controllers[i] = { false, false, false, false, false, false, false, false };
		controllermapping[i] = -1;
	}
	detect_controllers();
}

joypad::~joypad() {
	// close all detected controllers.
	for (auto gameController : gameControllers) {
		SDL_GameControllerClose(gameController);
	}
}

void	joypad::set_default_configs() {
	cfg[0] = { 0x50, 0x52, 0x4f, 0x51, 0x1d, 0x1b, 0x04, 0x16 };
	cfg[1] = { 0x0d, 0x0c, 0x0f, 0x0e, 0x14, 0x1a, 0x08, 0x15 };
	cfg_gc[0] = { 0x0d, 0x0b, 0x0e, 0x0c, 0x00, 0x02, 0x04, 0x06 };	// controller mappings defaults are actually the same.
	cfg_gc[1] = { 0x0d, 0x0b, 0x0e, 0x0c, 0x00, 0x02, 0x04, 0x06 };
	cfg_gc[2] = { 0x0d, 0x0b, 0x0e, 0x0c, 0x00, 0x02, 0x04, 0x06 };
	cfg_gc[3] = { 0x0d, 0x0b, 0x0e, 0x0c, 0x00, 0x02, 0x04, 0x06 };
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

void	joypad::process_controller_connect_event(SDL_Event* event) {
	// basicly we are going to disconnect and reconnect.
	reset_controller_detect();
	detect_controllers();
	bool added = event->type == SDL_CONTROLLERDEVICEADDED;
	if (added) {
		std::cout << "Controller ID #" << std::dec << (int)event->cdevice.which << " connected..\n";
	}
	else {
		std::cout << "Controller ID #" << std::dec << (int)event->cdevice.which << " disconnected..\n";
	}
}

void	joypad::process_controller_event(SDL_Event *event) {
	bool buttonstate = (event->type == SDL_CONTROLLERBUTTONDOWN);
	bool axisstate = (event->type == SDL_CONTROLLERAXISMOTION);
	int  whatdevice = event->cdevice.which;
	int  keycode = event->cbutton.button;
	if (!axisstate) {
		std::cout << "Controller response ID: #" << std::dec << (int)whatdevice << "\n";
		for (int i = 0; i < MAX_CONTROLLERS; i++) {
			if (controllermapping[i] == whatdevice) {
				if (cfg_gc[i].left == keycode)
					controllers[i].states[1] = buttonstate;
				if (cfg_gc[i].right == keycode)
					controllers[i].states[0] = buttonstate;
				if (cfg_gc[i].up == keycode)
					controllers[i].states[3] = buttonstate;
				if (cfg_gc[i].down == keycode)
					controllers[i].states[2] = buttonstate;
				if (cfg_gc[i].a == keycode)
					controllers[i].states[7] = buttonstate;
				if (cfg_gc[i].b == keycode)
					controllers[i].states[6] = buttonstate;
				if (cfg_gc[i].select == keycode)
					controllers[i].states[5] = buttonstate;
				if (cfg_gc[i].start == keycode)
					controllers[i].states[4] = buttonstate;
			}
		}
	}
	else {
		// process axis.. treat left / right axises the same. (as DPAD)
		for (int i = 0; i < MAX_CONTROLLERS; i++) {
			if (controllermapping[i] == whatdevice) {
				switch (event->caxis.axis) {
				case SDL_CONTROLLER_AXIS_LEFTX:
					if (event->caxis.value <= -DEADZONE) {
						controllers[i].left = true;
					}
					else controllers[i].left = false;
					if (event->caxis.value >= DEADZONE) {
						controllers[i].right = true;
					}
					else controllers[i].right = false;
					break;
				case SDL_CONTROLLER_AXIS_LEFTY:
					if (event->caxis.value <= -DEADZONE) {
						controllers[i].up = true;
					}
					else controllers[i].up = false;
					if (event->caxis.value >= DEADZONE) {
						controllers[i].down = true;
					}
					else controllers[i].down = false;
					break;
				}
			}
		}
	}
}

void	joypad::strobe(int controller_id) {
	for (int i = 0; i < MAX_CONTROLLERS; i++) readouts[i] = 0x00;
}

bool	joypad::pulse_read_out(int controller_id) {
	if (readouts[controller_id] > 7) return 0;
	// force axis reads.
	if ((readouts[controller_id] == 7) && controllers[controller_id].right) {
		readouts[controller_id]++;
		return true;
	}
	if ((readouts[controller_id] == 6) && controllers[controller_id].left) {
		readouts[controller_id]++;
		return true;
	}
	if ((readouts[controller_id] == 5) && controllers[controller_id].down) { 
		readouts[controller_id]++;
		return true;
	}
	if ((readouts[controller_id] == 4) && controllers[controller_id].up) {
		readouts[controller_id]++;
		return true;
	}
	return controllers[controller_id].states[7-readouts[controller_id]++];
}

void	joypad::detect_controllers() {
	// enumerate all controllers.
	if (gameControllers.size() > 0) return;	// already detected.
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			std::cout << std::dec << (int)i << ". " << SDL_GameControllerNameForIndex(i) << "\n";
			gameControllers.push_back(SDL_GameControllerOpen(i));
		}
	}
}

void	joypad::reset_controller_detect() {
	// disconnect all controllers from the event system.
	for (SDL_GameController *controller : gameControllers) {
		SDL_GameControllerClose(controller);
	}
	// clear detection.
	gameControllers.clear();
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
